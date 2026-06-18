#include "tree_oak.h"
#include "gen2_noise.h"

int gen2_tree_oak_height(uint32_t seed) {
    // 4..6 trunk
    return 4 + (int)(voxl_gen2_hash2(0, 0, seed) % 3u);
}

int gen2_tree_oak_build(gen2_place_buf *out, int wx, int wy, int wz, uint32_t seed) {
    int n = 0;
    int trunk = gen2_tree_oak_height(seed);

    // trunk
    for (int i = 0; i < trunk; i++) {
        n += gen2_place_add(out, wx, wy + i, wz, BLOCK_WOOD);
    }

    // leaf blob: two stacked radius-2 disks, then a small cap
    int top = wy + trunk;
    for (int dy = -2; dy <= 1; dy++) {
        int r = (dy >= 0) ? 1 : 2;
        for (int dz = -r; dz <= r; dz++) {
            for (int dx = -r; dx <= r; dx++) {
                if (dx == 0 && dz == 0 && dy < 0) continue;   // keep trunk core
                if (dx * dx + dz * dz > r * r + 1) continue;
                // nibble corners randomly for a less blocky look
                if ((dx * dx + dz * dz) == r * r + 1 &&
                    (voxl_gen2_hash3(wx + dx, top + dy, wz + dz, seed) & 1u)) {
                    continue;
                }
                n += gen2_place_add(out, wx + dx, top + dy, wz + dz, BLOCK_LEAVES);
            }
        }
    }
    // single cap leaf
    n += gen2_place_add(out, wx, top + 2, wz, BLOCK_LEAVES);
    return n;
}
