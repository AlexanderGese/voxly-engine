#include "structure_well.h"
#include "gen2_noise.h"

int gen2_well_should_place(int chunk_x, int chunk_z, uint32_t seed) {
    // rarer than ruins, ~1 in 40 chunks
    uint32_t h = voxl_gen2_hash2(chunk_x, chunk_z, seed ^ 0x9E11u);
    return (h % 40u) == 0u;
}

int gen2_well_build(gen2_place_buf *out, int wx, int wy, int wz) {
    int n = 0;

    // 4x4 brick rim at floor level
    for (int dz = 0; dz < 4; dz++) {
        for (int dx = 0; dx < 4; dx++) {
            int edge = (dx == 0 || dx == 3 || dz == 0 || dz == 3);
            if (edge) {
                n += gen2_place_add(out, wx + dx, wy, wz + dz, BLOCK_BRICK);
            } else {
                // 2x2 water core, dug down two
                n += gen2_place_add(out, wx + dx, wy,     wz + dz, BLOCK_WATER);
                n += gen2_place_add(out, wx + dx, wy - 1, wz + dz, BLOCK_WATER);
                n += gen2_place_add(out, wx + dx, wy - 2, wz + dz, BLOCK_BRICK);
            }
        }
    }

    // four corner posts up 2
    static const int corners[4][2] = { {0,0}, {0,3}, {3,0}, {3,3} };
    for (int c = 0; c < 4; c++) {
        for (int h = 1; h <= 2; h++) {
            n += gen2_place_add(out, wx + corners[c][0], wy + h,
                                wz + corners[c][1], BLOCK_BRICK);
        }
    }

    // flat cobble roof over the top
    for (int dz = 0; dz < 4; dz++) {
        for (int dx = 0; dx < 4; dx++) {
            n += gen2_place_add(out, wx + dx, wy + 3, wz + dz, BLOCK_COBBLE);
        }
    }
    return n;
}
