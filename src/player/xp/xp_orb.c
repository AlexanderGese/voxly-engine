#include "xp_orb.h"

#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "xp_config.h"
#include "xp_tier.h"
#include "../../math/rng.h"

// the pool. orbs[] is fixed at XP_ORB_MAX. a free list threads through dead
// slots via `freelist` indices stored in a parallel array, which keeps the
// orb struct itself free of intrusive pointers (the renderer reads it).
struct xp_orb_pool {
    xp_orb  orbs[XP_ORB_MAX];
    int     next_free[XP_ORB_MAX]; // -1 terminated singly-linked free chain
    int     free_head;
    int     live;
    rng     rng;
};

xp_orb_pool *xp_orb_pool_create(uint64_t seed) {
    xp_orb_pool *p = calloc(1, sizeof *p);
    if (!p) return NULL;

    // thread the free list low-index-first so spawns are cache friendly.
    for (int i = 0; i < XP_ORB_MAX; i++) {
        p->orbs[i].alive = 0;
        p->next_free[i] = i + 1;
    }
    p->next_free[XP_ORB_MAX - 1] = -1;
    p->free_head = 0;
    p->live = 0;
    rng_init(&p->rng, seed ? seed : 0x0B1700D5u);
    return p;
}

void xp_orb_pool_destroy(xp_orb_pool *p) {
    free(p);
}

int xp_orb_capacity(const xp_orb_pool *p) { (void)p; return XP_ORB_MAX; }
int xp_orb_live_count(const xp_orb_pool *p) { return p->live; }

xp_orb *xp_orb_at(xp_orb_pool *p, int idx) {
    if (idx < 0 || idx >= XP_ORB_MAX) return NULL;
    return &p->orbs[idx];
}

int xp_orb_spawn(xp_orb_pool *p, vec3 pos, int tier) {
    if (p->free_head < 0) return -1; // full
    int idx = p->free_head;
    p->free_head = p->next_free[idx];

    xp_orb *o = &p->orbs[idx];
    memset(o, 0, sizeof *o);
    o->pos = pos;
    o->tier = tier;
    o->alive = 1;
    o->bob_phase = rng_frange(&p->rng, 0.0f, 6.2831853f);

    // random outward pop so a cluster spreads instead of stacking.
    float a = rng_frange(&p->rng, 0.0f, 6.2831853f);
    float sp = rng_frange(&p->rng, 0.4f, 1.0f) * XP_ORB_SPAWN_JITTER;
    o->vel = vec3_new(cosf(a) * sp, rng_frange(&p->rng, 1.5f, 3.5f), sinf(a) * sp);

    p->live++;
    return idx;
}

void xp_orb_kill(xp_orb_pool *p, int idx) {
    if (idx < 0 || idx >= XP_ORB_MAX) return;
    xp_orb *o = &p->orbs[idx];
    if (!o->alive) return; // idempotent
    o->alive = 0;
    p->next_free[idx] = p->free_head;
    p->free_head = idx;
    p->live--;
}

aabb xp_orb_bounds(const xp_orb *o) {
    float r = xp_tier_get(o->tier)->radius;
    vec3 half = vec3_new(r, r, r);
    return aabb_from_center(o->pos, half);
}

// approximate ground. we don't have the world here (keeps this module
// world-agnostic for tests) so orbs settle near their spawn floor using a
// soft floor at integer y. good enough; xp_collect grabs them long before
// the eye notices.
static float orb_floor(const xp_orb *o) {
    return floorf(o->pos.y) + 0.05f;
}

void xp_orb_pool_update(xp_orb_pool *p, vec3 target, float dt) {
    if (dt <= 0.0f) return;
    float mag_sq = XP_ORB_MAGNET_RANGE * XP_ORB_MAGNET_RANGE;

    for (int i = 0; i < XP_ORB_MAX; i++) {
        xp_orb *o = &p->orbs[i];
        if (!o->alive) continue;

        o->age += dt;
        o->bob_phase += dt * 3.0f;
        if (o->age > XP_ORB_LIFETIME) { xp_orb_kill(p, i); continue; }

        vec3 to = vec3_sub(target, o->pos);
        float d2 = vec3_length_sq(to);

        if (d2 < mag_sq) {
            // magnetize: accelerate toward player, scaled so close orbs snap
            // and far ones drift. once magnetized we stay that way.
            o->magnetized = 1;
            float d = sqrtf(d2);
            vec3 dir = d > 1e-4f ? vec3_scale(to, 1.0f / d) : VEC3_UP;
            float pull = XP_ORB_MAGNET_ACCEL * (1.0f - d / XP_ORB_MAGNET_RANGE * 0.5f);
            o->vel = vec3_add(o->vel, vec3_scale(dir, pull * dt));
        }

        if (!o->magnetized) {
            // free flight: gravity + drag, settle on the soft floor.
            o->vel.y += XP_ORB_GRAVITY * dt;
            float floor_y = orb_floor(o);
            if (o->pos.y <= floor_y && o->vel.y < 0.0f) {
                o->pos.y = floor_y;
                o->vel.y = -o->vel.y * XP_ORB_BOUNCE;
                // kill tiny bounces so they don't jitter forever.
                if (fabsf(o->vel.y) < 0.4f) o->vel.y = 0.0f;
                o->vel.x *= 0.6f;
                o->vel.z *= 0.6f;
            }
            // horizontal drag
            float drag = 1.0f - XP_ORB_DRAG * dt;
            if (drag < 0.0f) drag = 0.0f;
            o->vel.x *= drag;
            o->vel.z *= drag;
        }

        // clamp speed so magnetized orbs don't overshoot wildly.
        float sp2 = vec3_length_sq(o->vel);
        float maxsp = XP_ORB_MAX_SPEED;
        if (sp2 > maxsp * maxsp) {
            float s = maxsp / sqrtf(sp2);
            o->vel = vec3_scale(o->vel, s);
        }

        o->pos = vec3_add(o->pos, vec3_scale(o->vel, dt));
    }
}
