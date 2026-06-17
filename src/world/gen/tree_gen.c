#include "tree_gen.h"
#include "../block.h"
#include "../../config.h"

static void set_if_air(chunk *c, int x, int y, int z, block_id id) {
    if (x < 0 || x >= CHUNK_SIZE_X) return;
    if (z < 0 || z >= CHUNK_SIZE_Z) return;
    if (y < 0 || y >= CHUNK_SIZE_Y) return;
    if (chunk_get_block(c, x, y, z) == BLOCK_AIR)
        chunk_set_block(c, x, y, z, id);
}

static void gen_oak(chunk *c, int lx, int ly, int lz, rng *r) {
    int height = 4 + rng_range(r, 0, 2);
    if (ly + height + 2 >= CHUNK_SIZE_Y) return;
    if (lx < 2 || lx >= CHUNK_SIZE_X - 2) return;
    if (lz < 2 || lz >= CHUNK_SIZE_Z - 2) return;

    for (int i = 0; i < height; i++)
        chunk_set_block(c, lx, ly + i, lz, BLOCK_WOOD);

    for (int dy = height - 2; dy <= height + 1; dy++) {
        int r_leaf = (dy >= height) ? 1 : 2;
        for (int dx = -r_leaf; dx <= r_leaf; dx++) {
            for (int dz = -r_leaf; dz <= r_leaf; dz++) {
                if (dx == 0 && dz == 0 && dy < height) continue;
                if (dx * dx + dz * dz > r_leaf * r_leaf + 1) continue;
                set_if_air(c, lx + dx, ly + dy, lz + dz, BLOCK_LEAVES);
            }
        }
    }
}

static void gen_birch(chunk *c, int lx, int ly, int lz, rng *r) {
    int height = 5 + rng_range(r, 0, 2);
    if (ly + height + 2 >= CHUNK_SIZE_Y) return;
    if (lx < 2 || lx >= CHUNK_SIZE_X - 2) return;
    if (lz < 2 || lz >= CHUNK_SIZE_Z - 2) return;

    for (int i = 0; i < height; i++)
        chunk_set_block(c, lx, ly + i, lz, BLOCK_WOOD);

    for (int dy = height - 2; dy <= height; dy++) {
        int r_leaf = (dy == height) ? 1 : 2;
        for (int dx = -r_leaf; dx <= r_leaf; dx++) {
            for (int dz = -r_leaf; dz <= r_leaf; dz++) {
                if (dx == 0 && dz == 0 && dy < height) continue;
                set_if_air(c, lx + dx, ly + dy, lz + dz, BLOCK_LEAVES);
            }
        }
    }
}

static void gen_spruce(chunk *c, int lx, int ly, int lz, rng *r) {
    int height = 6 + rng_range(r, 0, 3);
    if (ly + height + 1 >= CHUNK_SIZE_Y) return;
    if (lx < 3 || lx >= CHUNK_SIZE_X - 3) return;
    if (lz < 3 || lz >= CHUNK_SIZE_Z - 3) return;

    for (int i = 0; i < height; i++)
        chunk_set_block(c, lx, ly + i, lz, BLOCK_WOOD);

    // conical canopy
    for (int dy = 2; dy <= height; dy++) {
        int r_leaf = (height - dy) / 2 + 1;
        if (r_leaf > 3) r_leaf = 3;
        for (int dx = -r_leaf; dx <= r_leaf; dx++) {
            for (int dz = -r_leaf; dz <= r_leaf; dz++) {
                if (dx == 0 && dz == 0) continue;
                if (dx * dx + dz * dz > r_leaf * r_leaf) continue;
                set_if_air(c, lx + dx, ly + dy, lz + dz, BLOCK_LEAVES);
            }
        }
    }
    set_if_air(c, lx, ly + height, lz, BLOCK_LEAVES);
}

static void gen_jungle(chunk *c, int lx, int ly, int lz, rng *r) {
    int height = 8 + rng_range(r, 0, 4);
    if (ly + height + 3 >= CHUNK_SIZE_Y) return;
    if (lx < 3 || lx >= CHUNK_SIZE_X - 3) return;
    if (lz < 3 || lz >= CHUNK_SIZE_Z - 3) return;

    // thick trunk (2x2)
    for (int i = 0; i < height; i++) {
        chunk_set_block(c, lx,     ly + i, lz,     BLOCK_WOOD);
        chunk_set_block(c, lx + 1, ly + i, lz,     BLOCK_WOOD);
        chunk_set_block(c, lx,     ly + i, lz + 1, BLOCK_WOOD);
        chunk_set_block(c, lx + 1, ly + i, lz + 1, BLOCK_WOOD);
    }

    // large spherical canopy
    for (int dy = height - 3; dy <= height + 2; dy++) {
        int r_leaf = 3;
        for (int dx = -r_leaf; dx <= r_leaf; dx++) {
            for (int dz = -r_leaf; dz <= r_leaf; dz++) {
                if (dx * dx + dz * dz + (dy - height) * (dy - height) > r_leaf * r_leaf + 2) continue;
                set_if_air(c, lx + dx, ly + dy, lz + dz, BLOCK_LEAVES);
            }
        }
    }
}

static void gen_acacia(chunk *c, int lx, int ly, int lz, rng *r) {
    int height = 4 + rng_range(r, 0, 2);
    if (ly + height + 3 >= CHUNK_SIZE_Y) return;
    if (lx < 3 || lx >= CHUNK_SIZE_X - 3) return;
    if (lz < 3 || lz >= CHUNK_SIZE_Z - 3) return;

    for (int i = 0; i < height; i++)
        chunk_set_block(c, lx, ly + i, lz, BLOCK_WOOD);

    // flat canopy
    for (int dx = -3; dx <= 3; dx++) {
        for (int dz = -3; dz <= 3; dz++) {
            if (dx * dx + dz * dz > 10) continue;
            set_if_air(c, lx + dx, ly + height,     lz + dz, BLOCK_LEAVES);
            set_if_air(c, lx + dx, ly + height + 1, lz + dz, BLOCK_LEAVES);
        }
    }
}

void tree_gen_place(chunk *c, int lx, int ly, int lz, tree_type type, rng *r) {
    switch (type) {
    case TREE_OAK:    gen_oak(c, lx, ly, lz, r);    break;
    case TREE_BIRCH:  gen_birch(c, lx, ly, lz, r);  break;
    case TREE_SPRUCE: gen_spruce(c, lx, ly, lz, r); break;
    case TREE_JUNGLE: gen_jungle(c, lx, ly, lz, r); break;
    case TREE_ACACIA: gen_acacia(c, lx, ly, lz, r); break;
    }
}
