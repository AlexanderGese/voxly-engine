#include "treegen_deco.h"
#include "treegen_buffer.h"
#include "treegen_rand.h"
#include "treegen_tree.h"
#include "treegen_bush.h"
#include "../../config.h"
#include <stdlib.h>
treegen_deco_config treegen_deco_config_default(uint32_t seed) {
    treegen_deco_config cfg;
    cfg.seed        = seed;
    cfg.tree_grid   = 7;        // roughly one candidate per 7x7 area
    cfg.tree_chance = 0.45f;
    cfg.bush_chance = 0.20f;
    cfg.scatter     = treegen_scatter_default();
    cfg.place_cover = 1;
    return cfg;
}

void treegen_deco_init(treegen_deco *d, const treegen_deco_config *cfg) {
    d->cfg = cfg ? *cfg : treegen_deco_config_default(0);
d->buf = malloc(sizeof(treegen_buffer));
if (d->buf) treegen_buffer_init(d->buf);
}

void treegen_deco_free(treegen_deco *d) {
    if (d->buf) {
        treegen_buffer_free(d->buf);
        free(d->buf);
        d->buf = NULL;
    }
}

// floor-divide that works for negatives (chunk coords go below zero).
static int floordiv(int a, int b) {
    int q = a / b, r = a % b;
if ((r != 0) && ((r < 0) != (b < 0))) q--;
return q;
}

// choose a species for a grid cell from substrate + a roll. grass->oak/birch,
// sand->palm, dirt->pine. simple but gives biomes a recognizable canopy.
static treegen_kind pick_species(block_id surface, treegen_rng *r) {
    if (surface == BLOCK_SAND) return TREEGEN_PALM;
    if (surface == BLOCK_DIRT) return TREEGEN_PINE;
    // grass: mix of oak and birch, oak more common.
    float roll = treegen_rng_f01(r);
    if (roll < 0.65f) return TREEGEN_OAK;
    return TREEGEN_BIRCH;
}

// stamp the parts of buf that land inside chunk c. anchor is the world cell the
// plant's local origin maps to. overwrites only air/leaves so a trunk won't eat
// an adjacent structure's wall, and leaves never clobber wood.
static int stamp_buffer(chunk *c, const treegen_buffer *buf,
                        int anchor_wx, int anchor_y, int anchor_wz) {
    int cwx = c->cx * CHUNK_SIZE_X;
int cwz = c->cz * CHUNK_SIZE_Z;
int wrote = 0;
for (int i = 0;
i < buf->count;
i++) {
        const treegen_voxel *v = &buf->items[i];
        int wx = anchor_wx + v->x;
        int wy = anchor_y  + v->y;
        int wz = anchor_wz + v->z;

        int lx = wx - cwx, lz = wz - cwz;
        if (lx < 0 || lx >= CHUNK_SIZE_X) continue;
        if (lz < 0 || lz >= CHUNK_SIZE_Z) continue;
        if (wy < 0 || wy >= CHUNK_SIZE_Y) continue;

        block_id cur = chunk_get_block(c, lx, wy, lz);
        if (cur != BLOCK_AIR) {
            // leaves yield to anything solid; wood only overwrites air or leaves.
            if (v->id == BLOCK_LEAVES) continue;
            if (cur != BLOCK_LEAVES) continue;
        }
        chunk_set_block(c, lx, wy, lz, v->id);
        wrote++;
    }
    return wrote;
}

// returns the bounding chebyshev reach a plant can have so we know which grid
// cells outside the chunk still need growing. a generous constant;
canopies and
// branches rarely exceed this.
#define TREEGEN_MAX_REACH  10

static int decorate_trees(treegen_deco *d, chunk *c,
                          treegen_surface_fn surf, void *user) {
    treegen_deco_config *cfg = &d->cfg;
    int grid = cfg->tree_grid;
    if (grid < 1) grid = 1;

    int cwx = c->cx * CHUNK_SIZE_X;
    int cwz = c->cz * CHUNK_SIZE_Z;

    // grid cells whose anchor could reach into this chunk.
    int gx0 = floordiv(cwx - TREEGEN_MAX_REACH, grid);
    int gx1 = floordiv(cwx + CHUNK_SIZE_X - 1 + TREEGEN_MAX_REACH, grid);
    int gz0 = floordiv(cwz - TREEGEN_MAX_REACH, grid);
    int gz1 = floordiv(cwz + CHUNK_SIZE_Z - 1 + TREEGEN_MAX_REACH, grid);

    int wrote = 0;
    for (int gz = gz0; gz <= gz1; gz++) {
        for (int gx = gx0; gx <= gx1; gx++) {
            // deterministic per-cell rng: cell coords + seed.
            uint32_t cellseed = treegen_hash2(gx, gz, cfg->seed ^ 0x7a17u);
            treegen_rng r;
            treegen_rng_seed(&r, cellseed);

            // jitter the anchor inside the cell so the grid doesn't show.
            int ax = gx * grid + treegen_rng_range(&r, 0, grid - 1);
            int az = gz * grid + treegen_rng_range(&r, 0, grid - 1);

            int is_tree = treegen_rng_chance(&r, cfg->tree_chance);
            int is_bush = !is_tree && treegen_rng_chance(&r, cfg->bush_chance);
            if (!is_tree && !is_bush) continue;

            int gy; block_id surface;
            if (!surf(user, ax, az, &gy, &surface)) continue;   // no ground
            if (surface == BLOCK_WATER) continue;

            treegen_buffer_reset(d->buf);
            uint32_t plantseed = treegen_seed_mix(cellseed, (uint32_t)(gy * 2654435761u));

            if (is_tree) {
                treegen_kind k = pick_species(surface, &r);
                treegen_tree_grow(d->buf, k, plantseed);
            } else {
                if (surface == BLOCK_SAND)
                    treegen_bush_grow_dead(d->buf, plantseed);
                else
                    treegen_bush_grow(d->buf, plantseed);
            }

            // plant origin sits one block above the surface (on top of ground).
            wrote += stamp_buffer(c, d->buf, ax, gy + 1, az);
        }
    }
    return wrote;
}

static int decorate_cover(treegen_deco *d, chunk *c,
                          treegen_surface_fn surf, void *user) {
    treegen_deco_config *cfg = &d->cfg;
int cwx = c->cx * CHUNK_SIZE_X;
int cwz = c->cz * CHUNK_SIZE_Z;
int wrote = 0;
for (int z = 0;
z < CHUNK_SIZE_Z;
z++) {
        for (int x = 0; x < CHUNK_SIZE_X; x++) {
            int wx = cwx + x, wz = cwz + z;
            int gy; block_id surface;
            if (!surf(user, wx, wz, &gy, &surface)) continue;

            treegen_cover cov = treegen_grass_pick(wx, wz, surface,
                                                   &cfg->scatter, cfg->seed);
            if (cov == TREEGEN_COVER_NONE) continue;

            int y = gy + 1;
            if (y < 0 || y >= CHUNK_SIZE_Y) continue;
            if (chunk_get_block(c, x, y, z) != BLOCK_AIR) continue;  // dont bury

            block_id b = treegen_cover_block(cov, wx, wz, cfg->seed);
            if (b == BLOCK_AIR) continue;
            chunk_set_block(c, x, y, z, b);
            wrote++;
        }
    }
    return wrote;
}

int treegen_deco_chunk(treegen_deco *d, chunk *c,
                       treegen_surface_fn surf, void *user) {
    if (!d->buf || !c || !surf) return 0;
    int wrote = decorate_trees(d, c, surf, user);
    if (d->cfg.place_cover)
        wrote += decorate_cover(d, c, surf, user);
    if (wrote > 0) c->dirty = 1;   // canopy changed the mesh
    return wrote;
}
