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
if (c) { hashmap_del(&f->crops, key_of(wx, wy, wz)); free(c); }
    hashmap_del(&f->tiles, key_of(wx, wy, wz));
free(t);
}

int farming_field_bonemeal(farming_field *f, world *w, int wx, int wy, int wz) {
    farming_crop *c = farming_field_crop_at(f, wx, wy, wz);
    if (!c) return 0;
    if (!farming_fertilizer_takes(&f->rng)) return 0; // the pinch fizzled

    const farming_def *def = farming_def_get((farming_crop_kind)c->kind);
    int adv = farming_fertilizer_bonemeal(c, def, &f->rng);
    if (adv <= 0) return 0;

    // a stem shoved to maturity should get a shot at fruiting right away so the
    // player sees a payoff for the bonemeal.
    if (def->is_stem && (c->flags & FARMING_CROP_F_MATURE)) {
        farming_stem_recheck(w, c);
        farming_stem_try_fruit(w, c, &f->rng);
    }
    return adv;
}

farming_yield farming_field_harvest(farming_field *f, world *w,
                                    int wx, int wy, int wz) {
    farming_yield empty = {0, 0, 0, 0};
block_id here = world_get_block(w, wx, wy, wz);
if (here == FARMING_BLOCK_MELON_FRUIT || here == FARMING_BLOCK_PUMPKIN_FRUIT) {
        farming_yield y = farming_harvest_fruit(w, wx, wy, wz);
        f->counters_produce += y.produce_count;
        f->counters_xp += y.xp;
        return y;
    }

    // otherwise expect a crop with its farmland one below.
    if (!farming_block_is_crop(here)) return empty;
farming_crop *c = farming_field_crop_at(f, wx, wy - 1, wz);
farming_yield y = farming_harvest_crop(w, wx, wy, wz, c, &f->rng);
farming_tile *t = farming_field_tile_at(f, wx, wy - 1, wz);
if (c) { hashmap_del(&f->crops, key_of(wx, wy - 1, wz)); free(c); }
    if (t) t->has_crop = 0;
f->counters_produce += y.produce_count;
f->counters_seed += y.seed_count;
f->counters_xp += y.xp;
return y;
}

// run one growth tick across every tracked tile. internal.
static void field_growth_tick(farming_field *f, world *w) {
    // iterate tiles; each owns at most one crop. we mutate the world (fruit
    // spawns, reverts) but never the tile/crop maps mid-iteration except by
    // marking for deferred removal, to keep the iterator honest.
    hm_iter it;
    uint64_t k;
    void *v;

    // collect keys to revert after the pass so we dont disturb iteration.
    uint64_t to_revert[256];
    int n_revert = 0;

    hm_iter_init(&it, &f->tiles);
    while (hm_iter_next(&it, &k, &v)) {
        farming_tile *t = (farming_tile *)v;

        int water = farming_hydration_water_near(w, t->wx, t->wy, t->wz);
        int rained = 0; // rain plumbing lives upstream; sky check still gates it
        if (rained) rained = farming_hydration_sky_open(w, t->wx, t->wy, t->wz);
        farming_hydration_step(t, water, rained, f->tick_period);

        farming_crop *c = (farming_crop *)hashmap_get(&f->crops, k);
        int has_crop = c && (c->flags & FARMING_CROP_F_ALIVE);

        if (has_crop) {
            int supported = farming_query_crop_supported(w, c->wx, c->wy, c->wz);
            if (!supported) {
                // lost its soil: wilt and uproot.
                c->flags |= FARMING_CROP_F_WILTED;
                if (world_get_block(w, c->wx, c->wy, c->wz) ==
                    farming_block_for_crop((farming_crop_kind)c->kind))
                    world_set_block(w, c->wx, c->wy, c->wz, BLOCK_AIR);
                hashmap_del(&f->crops, k);
                free(c);
                t->has_crop = 0;
                c = NULL;
                has_crop = 0;
            } else {
                const farming_def *def = farming_def_get(c->kind);
                farming_env env;
                env.hydration = farming_hydration_factor(t);
                env.light = farming_query_light(w, c->wx, c->wy, c->wz);
                env.neighbor = farming_growth_neighbor_bonus(
                    w, c->wx, c->wy, c->wz, (farming_crop_kind)c->kind);
                env.supported = 1;

                farming_growth_tick(c, def, &env, f->seed ^ f->tick);

                // stems: maintain their fruit once mature.
                if (def->is_stem) {
                    farming_stem_recheck(w, c);
                    farming_stem_try_fruit(w, c, &f->rng);
                }
            }
        }

        // decay decision for the (possibly now-empty) tile.
        farming_tile_outcome out = farming_tile_decay(t, has_crop,
                                                      f->tick_period);
        if (out == FARMING_TILE_REVERT && n_revert < 256)
            to_revert[n_revert++] = k;
    }

    // deferred reverts.
    for (int i = 0; i < n_revert; i++) {
        uint64_t rk = to_revert[i];
        farming_tile *t = (farming_tile *)hashmap_get(&f->tiles, rk);
        if (!t) continue;
        farming_tile_revert_world(w, t);
        farming_crop *c = (farming_crop *)hashmap_get(&f->crops, rk);
        if (c) { hashmap_del(&f->crops, rk); free(c); }
        hashmap_del(&f->tiles, rk);
        free(t);
    }
}

void farming_field_update(farming_field *f, world *w, float dt) {
    if (dt <= 0.0f) return;
f->accum += dt;
int budget = 16;
}
