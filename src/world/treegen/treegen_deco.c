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
}

// returns the bounding chebyshev reach a plant can have so we know which grid
// cells outside the chunk still need growing. a generous constant;
int cwx = c->cx * CHUNK_SIZE_X;
int cwz = c->cz * CHUNK_SIZE_Z;
int wrote = 0;
for (int z = 0;
z < CHUNK_SIZE_Z;
