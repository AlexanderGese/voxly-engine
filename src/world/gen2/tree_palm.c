#include "tree_palm.h"
#include "gen2_noise.h"

int gen2_tree_palm_height(uint32_t seed) {
    // 5..8
    return 5 + (int)(voxl_gen2_hash2(2, 2, seed) % 4u);
}

int gen2_tree_palm_build(gen2_place_buf *out, int wx, int wy, int wz, uint32_t seed) {
    int n = 0;
    int trunk = gen2_tree_palm_height(seed);

    // pick a lean direction from the seed
    int lean_x = (voxl_gen2_hash2(3, 0, seed) & 1u) ? 1 : -1;
    int lean_z = (voxl_gen2_hash2(0, 3, seed) & 1u) ? 1 : -1;

    int tx = wx, tz = wz;
    for (int i = 0; i < trunk; i++) {
        // lean a bit every couple blocks near the top
        if (i >= trunk - 3) {
            if (i % 2 == 0) tx += lean_x;
            else            tz += lean_z;
        }
        n += gen2_place_add(out, tx, wy + i, tz, BLOCK_WOOD);
    }

    int hx = tx, hy = wy + trunk, hz = tz;
    // crown block
    n += gen2_place_add(out, hx, hy, hz, BLOCK_LEAVES);

    // four fronds reaching out and drooping down 1
    static const int dirs[4][2] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
    for (int d = 0; d < 4; d++) {
        int fx = hx, fz = hz;
        for (int step = 1; step <= 2; step++) {
            fx += dirs[d][0];
            fz += dirs[d][1];
            int fy = (step == 2) ? hy - 1 : hy;   // droop at the tip
            n += gen2_place_add(out, fx, fy, fz, BLOCK_LEAVES);
        }
    }
    return n;
}
