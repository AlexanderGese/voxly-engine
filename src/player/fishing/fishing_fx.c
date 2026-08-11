#include "fishing_fx.h"
#include <stddef.h>

// find a free particle slot the same way fx/particles.c does internally. it
// doesn't expose alloc_slot, so we re-do the linear scan here. cheap enough.
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
    for (int i = 0; i < n; i++) {
        int k = fx_alloc(ps);
        if (k < 0) break;
        particle *p = &ps->list[k];

        p->pos = vec3_add(at, (vec3){
            fishing_rng_frange(r, -0.15f, 0.15f),
            fishing_rng_frange(r, -0.05f, 0.05f),
            fishing_rng_frange(r, -0.15f, 0.15f),
        });
        p->vel = (vec3){
            fishing_rng_frange(r, -spread, spread),
            fishing_rng_frange(r, up_bias * 0.4f, up_bias),
            fishing_rng_frange(r, -spread, spread),
        };
        p->age  = 0.0f;
        p->life = fishing_rng_frange(r, 0.35f, 0.8f);
        p->size = size;
        p->r = cr; p->g = cg; p->b = cb;
        p->alive = 1;
        ps->count++;
    }
}

void fishing_fx_splash(particle_system *ps, fishing_rng *r, vec3 at) {
    // a healthy crown of water, mostly upward.
    emit(ps, r, at, 10, 1.2f, 3.5f, 0.08f, 0.55f, 0.7f, 0.95f);
}

void fishing_fx_lure_ripple(particle_system *ps, fishing_rng *r, vec3 at, float intensity) {
    if (intensity < 0.0f) intensity = 0.0f;
    if (intensity > 1.0f) intensity = 1.0f;
    // a sparse low ring; count tracks how close the bite is.
    int n = 1 + (int)(intensity * 2.0f);
    emit(ps, r, at, n, 0.8f, 0.6f, 0.05f, 0.6f, 0.75f, 0.95f);
}

void fishing_fx_bite(particle_system *ps, fishing_rng *r, vec3 at) {
    // tight, sharp, a touch darker — the water dimples as it gets pulled under.
    emit(ps, r, at, 6, 0.5f, 1.5f, 0.06f, 0.4f, 0.55f, 0.8f);
}

void fishing_fx_land(particle_system *ps, fishing_rng *r, vec3 at, fishing_catch_category cat) {
    // treasure gets a brighter, bigger pop; junk barely a flicker.
    switch (cat) {
    case CATCH_TREASURE:
        emit(ps, r, at, 14, 1.4f, 4.0f, 0.10f, 0.95f, 0.85f, 0.4f);  // golden
        break;
    case CATCH_JUNK:
        emit(ps, r, at, 4, 0.6f, 1.0f, 0.05f, 0.45f, 0.45f, 0.4f);   // murky
        break;
    case CATCH_FISH:
    default:
        emit(ps, r, at, 8, 1.0f, 2.5f, 0.07f, 0.55f, 0.7f, 0.95f);
        break;
    }
}
