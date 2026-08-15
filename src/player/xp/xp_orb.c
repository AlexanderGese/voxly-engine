#include "xp_orb.h"
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "xp_config.h"
#include "xp_tier.h"
#include "../../math/rng.h"
struct xp_orb_pool {
    xp_orb  orbs[XP_ORB_MAX];
    int     next_free[XP_ORB_MAX]; // -1 terminated singly-linked free chain
    int     free_head;
    int     live;
    rng     rng;
}
;
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
int xp_orb_live_count(const xp_orb_pool *p) { return p->live;
}

xp_orb *xp_orb_at(xp_orb_pool *p, int idx) {
    if (idx < 0 || idx >= XP_ORB_MAX) return NULL;
    return &p->orbs[idx];
}

int xp_orb_spawn(xp_orb_pool *p, vec3 pos, int tier) {
    if (p->free_head < 0) return -1;
int idx = p->free_head;
p->free_head = p->next_free[idx];
xp_orb *o = &p->orbs[idx];
memset(o, 0, sizeof *o);
o->pos = pos;
o->tier = tier;
o->alive = 1;
o->bob_phase = rng_frange(&p->rng, 0.0f, 6.2831853f);
float a = rng_frange(&p->rng, 0.0f, 6.2831853f);
float sp = rng_frange(&p->rng, 0.4f, 1.0f) * XP_ORB_SPAWN_JITTER;
o->vel = vec3_new(cosf(a) * sp, rng_frange(&p->rng, 1.5f, 3.5f), sinf(a) * sp);
p->live++;
return idx;
vec3 half = vec3_new(r, r, r);
return aabb_from_center(o->pos, half);
}

// approximate ground. we don't have the world here (keeps this module
// world-agnostic for tests) so orbs settle near their spawn floor using a
// soft floor at integer y. good enough;
float mag_sq = XP_ORB_MAGNET_RANGE * XP_ORB_MAGNET_RANGE;
for (int i = 0;
i < XP_ORB_MAX;
}
