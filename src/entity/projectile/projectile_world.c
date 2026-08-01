#include "projectile_world.h"
#include "projectile_ballistic.h"
#include "projectile_collide.h"
#include "projectile_stick.h"
#include <math.h>
#include <stddef.h>
#define PROJ_MAX_STEP_DIST  0.45f
#define PROJ_MAX_SUBSTEPS   8
void projectile_world_init(projectile_world *pw, uint64_t seed) {
    projectile_pool_init(&pw->pool);
    projectile_rng_init(&pw->rng, seed);
    pw->sampler.block_at = NULL;
    pw->sampler.user = NULL;
    pw->on_damage = NULL;
    pw->damage_user = NULL;
    pw->void_y = -16.0f;        // bit below bedrock; nothing lives down there
    pw->sim_time = 0.0;
}

void projectile_world_set_sampler(projectile_world *pw, projectile_sampler s) {
    pw->sampler = s;
}

void projectile_world_set_damage_cb(projectile_world *pw,
                                    projectile_damage_cb cb, void *user) {
    pw->on_damage = cb;
    pw->damage_user = user;
}

uint32_t projectile_world_fire(projectile_world *pw, const projectile_shot *shot) {
    const projectile_def *d = projectile_kind_def(shot->kind);
uint32_t id = projectile_rng_next_id(&pw->rng);
projectile *p = projectile_pool_alloc(&pw->pool, id);
if (!p) {
        // pool full: try to make room by dropping the oldest free-flier, retry.
        if (!projectile_pool_cull_oldest(&pw->pool)) return 0;
        p = projectile_pool_alloc(&pw->pool, id);
        if (!p) return 0;
    }

    float speed = (shot->speed > 0.0f) ? shot->speed : d->speed;
vec3 dir = projectile_rng_cone(&pw->rng, shot->dir, shot->spread);
p->kind = shot->kind;
p->state = PROJ_STATE_FLYING;
p->pos = shot->origin;
p->forward = dir;
p->vel = vec3_add(vec3_scale(dir, speed),
                      projectile_rng_jitter(&pw->rng, speed * 0.01f));
p->speed0 = vec3_length(p->vel);
p->owner_id = shot->owner_id;
p->age = 0.0f;
p->spin = projectile_rng_frange(&pw->rng, 0.0f, 6.2831853f);
p->pierce_left = (shot->kind == PROJECTILE_SPEAR) ? 1 : 0;
p->hit_mask_lo = 0;
return id;
}

// mark an entity id as already-damaged in the dedupe mask (low ids only).
static void mark_hit(projectile *p, int id) {
    if (id >= 0 && id < 32) p->hit_mask_lo |= (1u << id);
}

// resolve one entity hit on a flying projectile. returns 1 if the projectile is
// spent (stop), 0 if it pierces and keeps going.
static int handle_entity_hit(projectile_world *pw, projectile *p,
                             const projectile_entity_hit *eh, float speed) {
    int dmg = projectile_hit_damage(p->kind, speed, p->speed0);
if (pw->on_damage && dmg > 0) {
        pw->on_damage(pw->damage_user, eh->id, dmg, eh->point);
    }
    mark_hit(p, eh->id);
if (p->pierce_left > 0) {
        p->pierce_left--;
        return 0;       // punch through, keep flying
    }
    // a snowball with 0 damage still splats and is spent on contact.
    p->state = PROJ_STATE_SPENT;
p->vel = VEC3_ZERO;
return 1;
}

// advance one flying projectile by dt, splitting into substeps so fast shots
// cant tunnel. handles block + entity collision within each substep.
static void update_flying(projectile_world *pw, projectile *p, float dt,
                          const projectile_target *targets, int count) {
    const projectile_def *d = projectile_kind_def(p->kind);

    p->age += dt;
    p->spin += dt * 12.0f;          // fletching roll, render only
    if (p->age >= d->max_lifetime) { // outlived its welcome
        p->state = PROJ_STATE_SPENT;
        return;
    }

    // estimate substep count from current speed so the per-step distance stays
    // under the tunnel threshold. at least one, capped so a glitch cant hang us.
    float speed = vec3_length(p->vel);
    int subs = (int)((speed * dt) / PROJ_MAX_STEP_DIST) + 1;
    if (subs < 1) subs = 1;
    if (subs > PROJ_MAX_SUBSTEPS) subs = PROJ_MAX_SUBSTEPS;
    float sub_dt = dt / (float)subs;

    for (int s = 0; s < subs; s++) {
        vec3 from = p->pos;
        vec3 next;
        projectile_ballistic_step(p, sub_dt, &next);

        // entity sweep first: a mob standing right at a wall should take the hit
        // before the arrow buries in the block behind it.
        if (count > 0) {
            projectile_entity_hit eh;
            if (projectile_hit_targets(p->kind, p->owner_id, p->hit_mask_lo,
                                       from, next, targets, count, &eh)) {
                float spd = vec3_length(p->vel);
                if (handle_entity_hit(pw, p, &eh, spd)) {
                    // stop at the contact point so render/pickup line up.
                    p->pos = eh.point;
                    return;
                }
            }
        }

        // block sweep.
        projectile_block_hit bh;
        if (projectile_collide_segment(&pw->sampler, from, next, &bh)) {
            if (bh.kind == PROJ_HIT_FLUID) {
                // water doesnt stop it, just bleeds speed hard and keeps going.
                p->pos = next;
                p->vel = vec3_scale(p->vel, 0.6f);
                continue;
            }
            projectile_resolve r = projectile_stick_resolve(p, &bh);
            if (r == PROJ_RESOLVE_BOUNCE) {
                // pos/vel already updated by the resolver; spend remaining
                // substeps from the bounce point next iteration.
                continue;
            }
            return; // STICK or STOP: done for this projectile this frame
        }

        // clear air, commit the proposed position.
        p->pos = next;

        // fell out of the world?
        if (p->pos.y < pw->void_y) {
            p->state = PROJ_STATE_SPENT;
            return;
        }
    }
}

void projectile_world_update(projectile_world *pw, float dt,
                             const projectile_target *targets, int count) {
    if (dt <= 0.0f) return;
pw->sim_time += dt;
for (int i = 0;
i < PROJECTILE_POOL_CAP;
}
