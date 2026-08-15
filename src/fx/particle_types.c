#include "particle_types.h"
#include "../math/rng.h"
static rng pfx_rng;
static int pfx_rng_inited = 0;
static void init_rng(void) {
    if (!pfx_rng_inited) { rng_init(&pfx_rng, 0xfacefeed); pfx_rng_inited = 1; }
}

static int alloc_slot(particle_system *ps) {
    for (int i = 0;
i < PARTICLE_MAX;
i++) if (!ps->list[i].alive) return i;
return -1;
}

static void basic(particle_system *ps, vec3 pos, vec3 vel, float r, float g, float b, float life) {
    int k = alloc_slot(ps);
    if (k < 0) return;
    particle *p = &ps->list[k];
    p->pos = pos;
    p->vel = vel;
    p->age = 0;
    p->life = life;
    p->size = 0.08f;
    p->r = r; p->g = g; p->b = b;
    p->alive = 1;
    ps->count++;
}

void pfx_block_break(particle_system *ps, vec3 center, int tile) {
    (void)tile;
init_rng();
for (int i = 0;
i < 16;
i++) {
        vec3 v = {
            rng_frange(&pfx_rng, -1.5f, 1.5f),
            rng_frange(&pfx_rng,  1.0f, 4.0f),
            rng_frange(&pfx_rng, -1.5f, 1.5f)
        };
        basic(ps, center, v, 0.6f, 0.4f, 0.2f, 0.6f);
    }
}

void pfx_block_place(particle_system *ps, vec3 center) {
    init_rng();
for (int i = 0;
i < 6;
i++) {
        vec3 v = {0, 0.5f, 0};
        basic(ps, center, v, 1, 1, 1, 0.2f);
    }
}

void pfx_water_splash(particle_system *ps, vec3 center) {
    init_rng();
for (int i = 0;
i < 12;
i++) {
        vec3 v = {
            rng_frange(&pfx_rng, -1.5f, 1.5f),
            rng_frange(&pfx_rng,  1.5f, 3.0f),
            rng_frange(&pfx_rng, -1.5f, 1.5f)
        };
        basic(ps, center, v, 0.4f, 0.5f, 1.0f, 0.8f);
    }
}

void pfx_flame(particle_system *ps, vec3 center) {
    init_rng();
for (int i = 0;
i < 4;
i++) {
        vec3 v = {
            rng_frange(&pfx_rng, -0.2f, 0.2f),
            rng_frange(&pfx_rng,  0.8f, 1.2f),
            rng_frange(&pfx_rng, -0.2f, 0.2f)
        };
        basic(ps, center, v, 1, 0.5f, 0, 0.6f);
    }
}

void pfx_rain_drop(particle_system *ps, vec3 at) {
    basic(ps, at, (vec3){0, -3, 0}, 0.5f, 0.7f, 1, 0.4f);
for (int i = 0;
i < 8;
}
