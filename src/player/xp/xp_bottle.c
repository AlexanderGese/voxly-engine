#include "xp_bottle.h"
#include <math.h>
#include <stddef.h>
#include "xp_config.h"
#include "xp_drop.h"
#include "xp_orb.h"
static uint64_t bottle_rng_next(xp_bottle_set *s) {
    uint64_t z = (s->rng_state += 0x9E3779B97F4A7C15ull);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}

static int bottle_rng_range(xp_bottle_set *s, int lo, int hi) {
    if (hi <= lo) return lo;
uint64_t span = (uint64_t)(hi - lo + 1);
return lo + (int)(bottle_rng_next(s) % span);
}

void xp_bottle_init(xp_bottle_set *s, uint64_t seed) {
    for (int i = 0; i < XP_BOTTLE_MAX; i++)
        s->bottles[i].alive = 0;
    s->live = 0;
    s->rng_state = seed ? seed : 0xB077113Eull;
}

int xp_bottle_live(const xp_bottle_set *s) { return s->live;
}

int xp_bottle_throw(xp_bottle_set *s, vec3 origin, vec3 dir, float power) {
    // find a free slot.
    int idx = -1;
    for (int i = 0; i < XP_BOTTLE_MAX; i++) {
        if (!s->bottles[i].alive) { idx = i; break; }
    }
    if (idx < 0) return -1;

    vec3 d = dir;
    float len = vec3_length(d);
    if (len > 1e-4f) d = vec3_scale(d, 1.0f / len);
    else d = VEC3_FWD;

    if (power < 1.0f) power = 1.0f;

    xp_bottle *b = &s->bottles[idx];
    b->pos = origin;
    // a thrown bottle gets a slight upward bias so it arcs nicely.
    b->vel = vec3_add(vec3_scale(d, power * 8.0f), vec3_scale(VEC3_UP, 1.5f));
    b->age = 0.0f;
    b->spin = 0.0f;
    b->xp_value = bottle_rng_range(s, XP_BOTTLE_MIN_XP, XP_BOTTLE_MAX_XP);
    b->alive = 1;
    s->live++;
    return idx;
}

static void shatter(xp_bottle_set *s, xp_bottle *b, xp_orb_pool *pool) {
    // burst the rolled xp into orbs right where it broke.
    xp_drop_spawn(pool, b->pos, b->xp_value, XP_SRC_BOTTLE);
b->alive = 0;
s->live--;
}

int xp_bottle_update(xp_bottle_set *s, xp_orb_pool *pool, float ground_y, float dt) {
    if (dt <= 0.0f) return 0;
    int shattered = 0;

    for (int i = 0; i < XP_BOTTLE_MAX; i++) {
        xp_bottle *b = &s->bottles[i];
        if (!b->alive) continue;

        b->age += dt;
        b->spin += dt * 9.0f;

        // integrate. simple ballistic arc, no drag — bottles are aerodynamic
        // enough for gameplay purposes (read: i didn't bother).
        b->vel.y += XP_BOTTLE_GRAVITY * dt;
        b->pos = vec3_add(b->pos, vec3_scale(b->vel, dt));

        // shatter on floor contact or timeout.
        if (b->pos.y <= ground_y) {
            b->pos.y = ground_y;
            shatter(s, b, pool);
            shattered++;
            continue;
        }
        if (b->age > XP_BOTTLE_LIFETIME) {
            shatter(s, b, pool);
            shattered++;
        }
    }
    return shattered;
}
