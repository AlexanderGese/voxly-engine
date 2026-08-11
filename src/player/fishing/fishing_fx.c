#include "fishing_fx.h"
#include <stddef.h>
static int fx_alloc(particle_system *ps) {
    for (int i = 0; i < PARTICLE_MAX; i++)
        if (!ps->list[i].alive) return i;
    return -1;
}

// shared spawner: n droplets at `at` with a velocity cone and a water-ish tint.
// up_bias pushes the average velocity upward (a splash) vs sideways (a ripple).
static void emit(particle_system *ps, fishing_rng *r, vec3 at, int n,
                 float spread, float up_bias, float size,
                 float cr, float cg, float cb) {
    if (!ps) return;
for (int i = 0;
i < n;
