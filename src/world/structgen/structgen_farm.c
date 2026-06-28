#include "structgen_piece.h"

// farm plot and the village well. small enough to share one file.

// tilled dirt plot, a low wood fence ring, and a water channel down the middle
// so it reads as irrigated. crops would go here if we had a crop block; for now
// the dirt + water is the structure.
int structgen_build_farm(const structgen_piece *pc, structgen_buffer *out) {
    structgen_rng rng;
    structgen_rng_seed(&rng, pc->seed);

    structgen_box fp = pc->footprint;
    int y0 = fp.y0;
    int n = 0;

    // soil bed at y0. central column is a water channel.
    int cx = fp.x0 + structgen_box_width(&fp) / 2;
    for (int z = fp.z0; z < fp.z1; z++) {
        for (int x = fp.x0; x < fp.x1; x++) {
            block_id soil = (x == cx) ? BLOCK_WATER : BLOCK_DIRT;
            n += structgen_buffer_add(out, x, y0, z, soil);
        }
    }

    // fence: a one-tall wood frame around the perimeter, with random gaps so it
    // looks worn rather than machine-built.
    for (int z = fp.z0; z < fp.z1; z++) {
        for (int x = fp.x0; x < fp.x1; x++) {
            int edge = (x == fp.x0 || x == fp.x1 - 1 ||
                        z == fp.z0 || z == fp.z1 - 1);
            if (!edge) continue;
            if (structgen_rng_chance(&rng, 0.2f)) continue;  // missing post
            n += structgen_buffer_add(out, x, y0 + 1, z, BLOCK_WOOD);
        }
    }
    return n;
}

// the well: cobble rim, water core sunk a few blocks, two posts and a beam over
// the top. the classic. centerpiece of the village layout.
int structgen_build_well(const structgen_piece *pc, structgen_buffer *out) {
    structgen_box fp = pc->footprint;
    int y0 = fp.y0;
    int n = 0;

    // rim: a 3x3 (or whatever footprint) cobble frame at ground level.
    n += structgen_buffer_fill_frame(out, fp, y0, BLOCK_COBBLE);

    // water column going down from y0-1 a few blocks.
    int cx = fp.x0 + structgen_box_width(&fp) / 2;
    int cz = fp.z0 + structgen_box_depth(&fp) / 2;
    for (int dy = 1; dy <= 3; dy++)
        n += structgen_buffer_add(out, cx, y0 - dy, cz, BLOCK_WATER);

    // two corner posts + a roof beam, so it has the little shelter on top.
    int top = y0 + 3;
    for (int h = 1; h <= 2; h++) {
        n += structgen_buffer_add(out, fp.x0, y0 + h, fp.z0, BLOCK_WOOD);
        n += structgen_buffer_add(out, fp.x1 - 1, y0 + h, fp.z1 - 1, BLOCK_WOOD);
    }
    structgen_box beam = structgen_box_make(fp.x0, top, fp.z0, fp.x1, top + 1, fp.z1);
    n += structgen_buffer_fill_box(out, beam, BLOCK_PLANKS);
    return n;
}
