#include "structgen_piece.h"

// ruin colonnade: a rectangular stone-brick platform studded with broken
// pillars. height of each pillar is randomized and the tops are weathered so it
// reads as collapsed. this is the surface-decoration cousin of the dungeon room.

int structgen_build_pillar_hall(const structgen_piece *pc, structgen_buffer *out) {
    structgen_rng rng;
    structgen_rng_seed(&rng, pc->seed);

    structgen_box fp = pc->footprint;
    int y0 = fp.y0;
    int n = 0;

    // cracked platform: brick floor with a few tiles missing.
    for (int z = fp.z0; z < fp.z1; z++) {
        for (int x = fp.x0; x < fp.x1; x++) {
            if (structgen_rng_chance(&rng, 0.18f)) continue;   // broken slab
            block_id b = structgen_rng_chance(&rng, 0.3f) ? BLOCK_COBBLE : BLOCK_BRICK;
            n += structgen_buffer_add(out, x, y0, z, b);
        }
    }

    // pillars on a regular 2-block lattice inset from the edge. each one a
    // random surviving height, tapering with weathering near the top.
    int max_h = 4 + structgen_rng_range(&rng, 0, 2);
    for (int z = fp.z0 + 1; z < fp.z1 - 1; z += 2) {
        for (int x = fp.x0 + 1; x < fp.x1 - 1; x += 2) {
            int ph = structgen_rng_range(&rng, 1, max_h);
            for (int h = 1; h <= ph; h++) {
                // the top third of a pillar can be crumbled away block by block.
                if (h > (ph * 2) / 3 && structgen_rng_chance(&rng, 0.4f))
                    continue;
                n += structgen_buffer_add(out, x, y0 + h, z, BLOCK_BRICK);
            }
        }
    }

    // one taller intact pillar somewhere, picked from the lattice, for a focal
    // point. just rerun a tall stack at a hashed lattice node.
    int lx = fp.x0 + 1 + 2 * structgen_rng_range(&rng, 0, (structgen_box_width(&fp) - 2) / 2);
    int lz = fp.z0 + 1 + 2 * structgen_rng_range(&rng, 0, (structgen_box_depth(&fp) - 2) / 2);
    for (int h = 1; h <= max_h + 2; h++)
        n += structgen_buffer_add(out, lx, y0 + h, lz, BLOCK_BRICK);

    return n;
}
