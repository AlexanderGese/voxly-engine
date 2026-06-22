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

    // target voxel count.
    v.size = oregen_rng_range(&rr, ore->size_min, ore->size_max);
    if (v.size < 1) v.size = 1;

    // base radius for blob/pocket scales off the cube root of the size, so
    // a 20-voxel blob isnt tiny and a 4-voxel pocket isnt huge. veins use
    // it as a starting thickness.
    float r = 0.62f + 0.40f * (float)v.size;
    // crude cbrt without pulling in math.h: two newton steps off a guess.
    float g = 1.0f;
    for (int it = 0; it < 6; it++) g = (2.0f * g + r / (g * g)) / 3.0f;
    v.radius = g * 0.75f + oregen_rng_frange(&rr, -0.15f, 0.25f);
    if (v.radius < 0.6f) v.radius = 0.6f;

    return v;
}

int oregen_seed_chunk(oregen_vein *out, int out_cap,
                      int origin_x, int origin_z, int max_y,
                      uint32_t world_seed) {
    if (!out || out_cap <= 0) return 0;

    int chunk_x = origin_x >> 4;   // 16-wide columns, same as CHUNK_SIZE_X
    int chunk_z = origin_z >> 4;
    int n = 0;

    int kinds = oregen_table_count();
    for (int k = 0; k < kinds && n < out_cap; k++) {
        const oregen_ore *ore = oregen_table_at(k);
        uint32_t ks = oregen_seed_for(chunk_x, chunk_z, k, world_seed);

        oregen_rng rr;
        oregen_rng_seed(&rr, ks);

        // tries_per_chunk is an expected count. take the integer part for
        // sure, then roll the fractional remainder as a bonus attempt. this
        // is the cheap way to get a fractional rate without a poisson draw.
        float rate  = ore->tries_per_chunk;
        int   tries = (int)rate;
        if (oregen_rng_chance(&rr, rate - (float)tries)) tries++;
        if (tries <= 0) continue;

        // stratify the vein columns so two veins of the same ore dont stack
        // on the same spot. one scatter pass per ore per chunk.
        oregen_point pts[16];
        int np = oregen_scatter_grid(pts, 16, tries,
                                     CHUNK_SIZE_X, CHUNK_SIZE_Z,
                                     ks ^ 0x9d7f0a31u);
        if (np <= 0) continue;

        for (int a = 0; a < np && n < out_cap; a++) {
            int lx = pts[a].lx;
            int lz = pts[a].lz;

            int ymin = ore->y_min;
            int ymax = ore->y_max < max_y ? ore->y_max : max_y;
            if (ymax <= ymin) continue;

            int sy = oregen_rng_range(&rr, ymin, ymax);

            // soft band edges: accept proportional to the curve weight at
            // this y. rejected rolls just dont place, which thins the band
            // boundaries naturally.
            float w = oregen_curve_weight(ore, sy);
            if (!oregen_rng_chance(&rr, w)) continue;

            // per-vein seed so the blob walker is independent of how many
            // attempts came before it.
            uint32_t vs = oregen_seed_mix(ks, oregen_hash3(lx, sy, lz, ks + (uint32_t)a));

            out[n++] = oregen_seed_roll(k, origin_x + lx, sy, origin_z + lz, vs);
        }
    }

    return n;
}
