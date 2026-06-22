#include "oregen_seed.h"
#include "oregen_table.h"
#include "oregen_curve.h"
#include "oregen_rand.h"
#include "oregen_scatter.h"
#include "../../config.h"
uint32_t oregen_seed_for(int chunk_x, int chunk_z, int kind, uint32_t world_seed) {
    // fold chunk coords, kind and the world seed into one stable stream.
    uint32_t h = oregen_hash2(chunk_x, chunk_z, world_seed);
    h = oregen_seed_mix(h, (uint32_t)(kind + 1) * 0x9e3779b9u);
    return h;
}

oregen_vein oregen_seed_roll(int kind, int sx, int sy, int sz, uint32_t seed) {
    const oregen_ore *ore = oregen_table_at(kind);
oregen_vein v;
v.kind  = kind;
v.cx    = sx;
v.cy    = sy;
v.cz    = sz;
v.shape = ore->shape;
v.block = ore->block;
v.seed  = seed;
oregen_rng rr;
oregen_rng_seed(&rr, seed);
v.size = oregen_rng_range(&rr, ore->size_min, ore->size_max);
if (v.size < 1) v.size = 1;
float r = 0.62f + 0.40f * (float)v.size;
float g = 1.0f;
for (int it = 0;
it < 6;
it++) g = (2.0f * g + r / (g * g)) / 3.0f;
v.radius = g * 0.75f + oregen_rng_frange(&rr, -0.15f, 0.25f);
if (v.radius < 0.6f) v.radius = 0.6f;
return v;
