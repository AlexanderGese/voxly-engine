#ifndef WORLD_FARMING_FIELD_H
#define WORLD_FARMING_FIELD_H

#include "../world.h"
#include "../../util/hashmap.h"
#include "farming_types.h"
#include "farming_rng.h"

// the field manager. owns every tracked farmland tile and crop in the world,
// keyed by a packed block coordinate, and drives the slow growth tick. this is
// the one stateful object the rest of the engine pokes: till here, plant there,
// step it each tick, harvest on click.
//
// tiles and crops live in two parallel hashmaps keyed the same way (by the
// farmland block coord). a crop's key is its *farmland* coord, not the plant
// coord, so the two always line up by the same hash.

typedef struct {
    hashmap tiles;       // key -> farming_tile* (owned)
    hashmap crops;       // key -> farming_crop* (owned)
    uint32_t seed;       // world seed, folded into growth/yield rolls
    uint32_t tick;       // monotonically increasing growth tick counter
    float    accum;      // seconds banked toward the next growth tick
    float    tick_period;// seconds per growth tick
    farming_rng rng;     // shared stream for harvest rolls etc.
    int      counters_produce; // lifetime tallies, handy for hud/debug
    int      counters_seed;
    int      counters_xp;
} farming_field;

void farming_field_init(farming_field *f, uint32_t world_seed);
void farming_field_free(farming_field *f);

// till + register a farmland tile. returns 1 if new farmland was made.
int  farming_field_till(farming_field *f, world *w, int wx, int wy, int wz);

// plant a crop on existing farmland at the block above (wx,wy,wz is farmland).
// returns 1 on success.
int  farming_field_plant(farming_field *f, world *w, int wx, int wy, int wz,
                         farming_crop_kind kind);

// harvest whatever crop/fruit is at (wx,wy,wz). returns the yield (zeros if
// nothing harvestable). tallies into the field counters.
farming_yield farming_field_harvest(farming_field *f, world *w,
                                    int wx, int wy, int wz);

// register a trample event on the farmland at (wx,wy,wz).
void farming_field_trample(farming_field *f, world *w, int wx, int wy, int wz);

// apply bonemeal to the crop above the farmland at (wx,wy,wz). advances stages
// and, if the crop matures into a stem, lets it fruit immediately. returns the
// number of stages advanced (0 if nothing happened). the crop block is logical
// so theres nothing to re-place; stage lives in the crop record.
int  farming_field_bonemeal(farming_field *f, world *w, int wx, int wy, int wz);

// advance the whole field by dt seconds. runs growth ticks as time accrues.
void farming_field_update(farming_field *f, world *w, float dt);

// lookups (may return NULL). key is the farmland block coord.
farming_tile *farming_field_tile_at(farming_field *f, int wx, int wy, int wz);
farming_crop *farming_field_crop_at(farming_field *f, int wx, int wy, int wz);

// how many tiles / crops are currently tracked.
size_t farming_field_tile_count(const farming_field *f);
size_t farming_field_crop_count(const farming_field *f);

#endif
