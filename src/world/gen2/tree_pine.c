#include "tree_pine.h"
#include "gen2_noise.h"

int gen2_tree_pine_height(uint32_t seed) {
    // pines are tall: 7..11
    return 7 + (int)(voxl_gen2_hash2(1, 1, seed) % 5u);
}

int gen2_tree_pine_build(gen2_place_buf *out, int wx, int wy, int wz, uint32_t seed) {
    int n = 0;
    int trunk = gen2_tree_pine_height(seed);

    for (int i = 0; i < trunk; i++) {
        n += gen2_place_add(out, wx, wy + i, wz, BLOCK_WOOD);
    }

    // conical leaves: start near 1/3 up, radius shrinks toward the top.
    int leaf_start = trunk / 3;
    int layers = trunk - leaf_start + 1;
    for (int li = 0; li < layers; li++) {
        int ly = wy + leaf_start + li;
        // radius 2 at bottom -> 0 at top, with a wobble for ragged edges
        int r = 2 - (li * 2) / (layers > 0 ? layers : 1);
        if (r < 0) r = 0;
        for (int dz = -r; dz <= r; dz++) {
            for (int dx = -r; dx <= r; dx++) {
                if (dx == 0 && dz == 0) continue;   // trunk core stays wood
                if (dx * dx + dz * dz > r * r + 1) continue;
                n += gen2_place_add(out, wx + dx, ly, wz + dz, BLOCK_LEAVES);
            }
        }
    }
    // pointy tip
    n += gen2_place_add(out, wx, wy + trunk, wz, BLOCK_LEAVES);
    return n;
}
