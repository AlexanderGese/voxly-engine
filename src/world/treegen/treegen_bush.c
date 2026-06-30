#include "treegen_bush.h"
#include "treegen_rand.h"

// bushes are hand-built from offset leaf blobs. no turtle, no grammar; the shape
// is small enough that a couple of overlapping spheres look better than anything
// procedural would. all rolls go through the plant rng so they're stable.

int treegen_bush_grow(treegen_buffer *out, uint32_t seed) {
    int before = out->count;
    treegen_rng r;
    treegen_rng_seed(&r, seed);

    int stem = treegen_rng_range(&r, 0, 1);          // 0 or 1 block of wood
    if (stem > 0)
        treegen_buffer_column(out, 0, 0, stem, 0, 0, BLOCK_WOOD);

    int base_y = stem + 1;
    int lobes = treegen_rng_range(&r, 2, 3);
    int rad   = treegen_rng_range(&r, 1, 2);

    // central lobe, then a couple offset ones to break the silhouette.
    treegen_buffer_blob(out, 0, base_y, 0, rad, BLOCK_LEAVES, 78, &r);
    for (int i = 0; i < lobes; i++) {
        int ox = treegen_rng_range(&r, -1, 1);
        int oz = treegen_rng_range(&r, -1, 1);
        int oy = treegen_rng_range(&r,  0, 1);
        treegen_buffer_blob(out, ox, base_y + oy, oz,
                            rad > 1 ? rad - 1 : 1, BLOCK_LEAVES, 70, &r);
    }
    return out->count - before;
}

int treegen_bush_grow_dead(treegen_buffer *out, uint32_t seed) {
    int before = out->count;
    treegen_rng r;
    treegen_rng_seed(&r, seed);

    // a short central stick plus 2-4 twigs leaning out one block. all wood.
    int h = treegen_rng_range(&r, 1, 2);
    treegen_buffer_column(out, 0, 0, h, 0, 0, BLOCK_WOOD);

    int twigs = treegen_rng_range(&r, 2, 4);
    for (int i = 0; i < twigs; i++) {
        int dx = treegen_rng_range(&r, -1, 1);
        int dz = treegen_rng_range(&r, -1, 1);
        if (dx == 0 && dz == 0) dx = 1;              // dont stack on the stem
        treegen_buffer_add(out, dx, h, dz, BLOCK_WOOD);
        if (treegen_rng_chance(&r, 0.5f))            // some twigs reach higher
            treegen_buffer_add(out, dx, h + 1, dz, BLOCK_WOOD);
    }
    return out->count - before;
}

int treegen_bush_grow_capped(treegen_buffer *out, uint32_t seed, block_id cap) {
    int before = out->count;
    treegen_rng r;
    treegen_rng_seed(&r, seed);

    int base_y = 1;
    treegen_buffer_add(out, 0, 0, 0, BLOCK_WOOD);    // tiny stem
    int rad = treegen_rng_range(&r, 1, 2);
    treegen_buffer_blob(out, 0, base_y, 0, rad, BLOCK_LEAVES, 80, &r);

    // a few cap blocks scattered on the upper rind, like berries/blossoms.
    if (cap != BLOCK_AIR) {
        int berries = treegen_rng_range(&r, 2, 4);
        for (int i = 0; i < berries; i++) {
            int dx = treegen_rng_range(&r, -rad, rad);
            int dz = treegen_rng_range(&r, -rad, rad);
            treegen_buffer_add(out, dx, base_y + rad, dz, cap);
        }
    }
    return out->count - before;
}
