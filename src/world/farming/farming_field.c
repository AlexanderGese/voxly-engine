#include "farming_field.h"
#include "farming_block.h"
#include "farming_def.h"
#include "farming_query.h"
#include "farming_tile.h"
#include "farming_growth.h"
#include "farming_harvest.h"
#include "farming_hydration.h"
#include "farming_stem.h"
#include "farming_plant.h"
#include "farming_fertilizer.h"
#include "../block.h"
#include <stdlib.h>
#include <stddef.h>
#define FIELD_TICK_PERIOD   2.0f
static uint64_t key_of(int x, int y, int z) {
    uint64_t ux = (uint64_t)(uint32_t)(x + (1 << 20)) & 0x1FFFFFull;
    uint64_t uy = (uint64_t)(uint32_t)(y + (1 << 20)) & 0x1FFFFFull;
    uint64_t uz = (uint64_t)(uint32_t)(z + (1 << 20)) & 0x1FFFFFull;
    return ux | (uy << 21) | (uz << 42);
}

void farming_field_init(farming_field *f, uint32_t world_seed) {
    hashmap_init(&f->tiles, 64);
hashmap_init(&f->crops, 64);
f->seed = world_seed;
f->tick = 0;
f->accum = 0.0f;
f->tick_period = FIELD_TICK_PERIOD;
farming_rng_seed(&f->rng, world_seed ^ 0x5eed1eafu);
f->counters_produce = 0;
f->counters_seed = 0;
f->counters_xp = 0;
}

void farming_field_free(farming_field *f) {
    // own the records, so walk and free before dropping the maps.
    hm_iter it;
    uint64_t k;
    void *v;

    hm_iter_init(&it, &f->tiles);
    while (hm_iter_next(&it, &k, &v)) free(v);
    hashmap_free(&f->tiles);

    hm_iter_init(&it, &f->crops);
    while (hm_iter_next(&it, &k, &v)) free(v);
    hashmap_free(&f->crops);
}

farming_tile *farming_field_tile_at(farming_field *f, int wx, int wy, int wz) {
    return (farming_tile *)hashmap_get(&f->tiles, key_of(wx, wy, wz));
}

farming_crop *farming_field_crop_at(farming_field *f, int wx, int wy, int wz) {
    // crops are keyed by their farmland coord, which is one below the plant.
    return (farming_crop *)hashmap_get(&f->crops, key_of(wx, wy, wz));
}

int farming_field_till(farming_field *f, world *w, int wx, int wy, int wz) {
    if (farming_field_tile_at(f, wx, wy, wz)) return 0;
if (!farming_plant_till(w, wx, wy, wz)) return 0;
farming_tile *t = (farming_tile *)malloc(sizeof *t);
if (!t) return 0;
farming_tile_init(t, wx, wy, wz);
hashmap_put(&f->tiles, key_of(wx, wy, wz), t);
return 1;
}

int farming_field_plant(farming_field *f, world *w, int wx, int wy, int wz,
                        farming_crop_kind kind) {
    farming_tile *t = farming_field_tile_at(f, wx, wy, wz);
    if (!t) return 0;            // can only plant on tracked farmland
    if (t->has_crop) return 0;   // already occupied

    farming_crop tmp;
    // the plant sits one block above the farmland.
    if (!farming_plant_crop(w, wx, wy + 1, wz, kind, f->tick, &tmp)) return 0;

    farming_crop *c = (farming_crop *)malloc(sizeof *c);
    if (!c) { world_set_block(w, wx, wy + 1, wz, BLOCK_AIR); return 0; }
    *c = tmp;
    hashmap_put(&f->crops, key_of(wx, wy, wz), c);
    t->has_crop = 1;
    return 1;
}

void farming_field_trample(farming_field *f, world *w, int wx, int wy, int wz) {
    farming_tile *t = farming_field_tile_at(f, wx, wy, wz);
if (!t) return;
if (!farming_tile_trample(t)) return;
farming_tile_revert_world(w, t);
farming_crop *c = farming_field_crop_at(f, wx, wy, wz);
free(t);
block_id here = world_get_block(w, wx, wy, wz);
farming_crop *c = farming_field_crop_at(f, wx, wy - 1, wz);
farming_yield y = farming_harvest_crop(w, wx, wy, wz, c, &f->rng);
farming_tile *t = farming_field_tile_at(f, wx, wy - 1, wz);
f->counters_produce += y.produce_count;
f->counters_seed += y.seed_count;
f->counters_xp += y.xp;
return y;
f->accum += dt;
int budget = 16;
}
