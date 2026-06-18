#include "structure_ruin.h"
#include "gen2_noise.h"

int gen2_ruin_should_place(int chunk_x, int chunk_z, uint32_t seed) {
    // ~1 in 14 chunks
    uint32_t h = voxl_gen2_hash2(chunk_x, chunk_z, seed ^ 0x5211u);
    return (h % 14u) == 0u;
}

int gen2_ruin_build(gen2_place_buf *out, int wx, int wy, int wz, uint32_t seed) {
    int n = 0;
    int w = 5 + (int)(voxl_gen2_hash2(wx, wz, seed) % 3u);       // 5..7
    int d = 5 + (int)(voxl_gen2_hash2(wz, wx, seed + 1u) % 3u);  // 5..7
    int wall_h = 3;

    // cracked cobble floor
    for (int dz = 0; dz < d; dz++) {
        for (int dx = 0; dx < w; dx++) {
            // some floor tiles gone
            if (voxl_gen2_hash3(wx + dx, wy, wz + dz, seed ^ 0xF100u) % 4u == 0u)
                continue;
            n += gen2_place_add(out, wx + dx, wy, wz + dz, BLOCK_COBBLE);
        }
    }

    // perimeter walls, decayed: skip blocks at random and taper height
    for (int dz = 0; dz < d; dz++) {
        for (int dx = 0; dx < w; dx++) {
            int edge = (dx == 0 || dx == w - 1 || dz == 0 || dz == d - 1);
            if (!edge) continue;
            for (int h = 1; h <= wall_h; h++) {
                uint32_t r = voxl_gen2_hash3(wx + dx, wy + h, wz + dz, seed);
                // higher up == more likely missing (weathered top)
                uint32_t miss_chance = 2u + (uint32_t)h;   // out of ~6
                if ((r % 6u) < miss_chance && h >= 2) continue;
                if ((r % 5u) == 0u && h == 1) continue;     // gaps at base too
                n += gen2_place_add(out, wx + dx, wy + h, wz + dz, BLOCK_BRICK);
            }
        }
    }

    // a lone surviving pillar in one corner for flavor
    for (int h = 1; h <= wall_h + 1; h++) {
        n += gen2_place_add(out, wx + 1, wy + h, wz + 1, BLOCK_BRICK);
    }
    return n;
}
