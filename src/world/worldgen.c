#include "worldgen.h"
#include "block.h"
#include "../math/noise.h"
#include "../config.h"

#include <math.h>
#include <stdlib.h>

// little trick to reseed based on world+chunk coords so chunks are
// deterministic regardless of load order. seed is seeded once globally
// but grabbed per chunk via a cheap hash before noise sampling

static unsigned hash_u(unsigned x) {
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    x ^= x >> 16;
    return x;
}

int worldgen_height_at(int wx, int wz, unsigned seed) {
    (void)seed;
    float scale = 1.0f / 90.0f;
    float n  = noise_fbm2d(wx * scale,            wz * scale,            5, 2.0f, 0.5f);
    float nm = noise_fbm2d(wx * scale * 0.25f,    wz * scale * 0.25f,    3, 2.0f, 0.5f);

    float base = (float)WORLD_SEA_LEVEL + 6.0f;
    float h    = base + n * 18.0f + nm * 10.0f;
    int hi = (int)h;
    if (hi < 1) hi = 1;
    if (hi >= CHUNK_SIZE_Y - 1) hi = CHUNK_SIZE_Y - 2;
    return hi;
}

static void plant_tree(chunk *c, int lx, int ly, int lz) {
    if (ly + 6 >= CHUNK_SIZE_Y) return;
    if (lx < 2 || lx >= CHUNK_SIZE_X - 2) return;
    if (lz < 2 || lz >= CHUNK_SIZE_Z - 2) return;

    // trunk
    for (int i = 0; i < 5; i++) {
        chunk_set_block(c, lx, ly + i, lz, BLOCK_WOOD);
    }
    // leaves blob
    for (int dy = 3; dy <= 6; dy++) {
        int r = (dy == 6) ? 1 : 2;
        for (int dz = -r; dz <= r; dz++) {
            for (int dx = -r; dx <= r; dx++) {
                if (dx * dx + dz * dz > r * r + 1) continue;
                int x = lx + dx;
                int z = lz + dz;
                int y = ly + dy;
                if (x < 0 || x >= CHUNK_SIZE_X) continue;
                if (z < 0 || z >= CHUNK_SIZE_Z) continue;
                if (y < 0 || y >= CHUNK_SIZE_Y) continue;
                if (chunk_get_block(c, x, y, z) == BLOCK_AIR) {
                    chunk_set_block(c, x, y, z, BLOCK_LEAVES);
                }
            }
        }
    }
}

void worldgen_fill(chunk *c, unsigned seed) {
    // NOTE: do NOT reseed noise per chunk. the perlin perm table is GLOBAL
    // and reseeding it makes each chunk sample a different noise landscape,
    // destroying terrain continuity. the world seed is applied once at
    // startup (main.c calls noise_seed(seed) before any chunks load).
    // tree placement uses its own hash-based RNG below, independent of noise.
    (void)seed;

    for (int lz = 0; lz < CHUNK_SIZE_Z; lz++) {
        for (int lx = 0; lx < CHUNK_SIZE_X; lx++) {
            int wx = c->cx * CHUNK_SIZE_X + lx;
            int wz = c->cz * CHUNK_SIZE_Z + lz;

            int h = worldgen_height_at(wx, wz, seed);

            for (int y = 0; y <= h; y++) {
                block_id id;
                if (y == 0) id = BLOCK_BEDROCK;
                else if (y < h - 4) id = BLOCK_STONE;
                else if (y < h) id = BLOCK_DIRT;
                else id = (h < WORLD_SEA_LEVEL + 2) ? BLOCK_SAND : BLOCK_GRASS;
                chunk_set_block(c, lx, y, lz, id);
            }

            // water
            if (h < WORLD_SEA_LEVEL) {
                for (int y = h + 1; y <= WORLD_SEA_LEVEL; y++) {
                    chunk_set_block(c, lx, y, lz, BLOCK_WATER);
                }
            }

            // snow cap
            if (h > WORLD_SEA_LEVEL + 20) {
                chunk_set_block(c, lx, h + 1, lz, BLOCK_SNOW);
            }
        }
    }

    // scatter a few trees per chunk using cheap hash
    unsigned s = hash_u((unsigned)c->cx * 2654435761u + (unsigned)c->cz);
    int tries = 4 + (int)(s % 4);
    for (int t = 0; t < tries; t++) {
        s = hash_u(s);
        int lx = (int)(s % CHUNK_SIZE_X);
        s = hash_u(s);
        int lz = (int)(s % CHUNK_SIZE_Z);
        int wx = c->cx * CHUNK_SIZE_X + lx;
        int wz = c->cz * CHUNK_SIZE_Z + lz;
        int h = worldgen_height_at(wx, wz, seed);
        if (h < WORLD_SEA_LEVEL + 2) continue;
        if (chunk_get_block(c, lx, h, lz) != BLOCK_GRASS) continue;
        plant_tree(c, lx, h + 1, lz);
    }
}
