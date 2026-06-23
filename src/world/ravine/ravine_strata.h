#ifndef WORLD_RAVINE_STRATA_H
#define WORLD_RAVINE_STRATA_H

#include <stdint.h>
#include "ravine_types.h"

// the exposed-strata table. when a canyon wall cuts down through the rock it
// reveals horizontal bands — the whole point of the "exposed strata" look. we
// pre-roll a stack of ravine_band entries spanning from the bedrock guard up to
// somewhere above the surface, each band a thickness of one rock type with a
// hardness that biases the wall jitter (hard rock juts, soft rock recedes). the
// bands are keyed on *world y*, so the same band shows at the same height right
// along the canyon — that horizontal continuity is what sells the geology.

typedef struct {
    ravine_band bands[RAVINE_MAX_STRATA];
    int         count;
} ravine_strata;

// roll a strata table covering [floor_lo, top]. stream is the canyon's private
// rng seed so every region resolves identical bands for the same canyon.
void ravine_strata_build(ravine_strata *s, const ravine_params *p,
                         int floor_lo, int top, uint32_t stream);

// the band covering world y, or NULL if y falls outside the table.
const ravine_band *ravine_strata_band(const ravine_strata *s, int y);

// the exposed block at world y on a wall face, with the strata_jitter wobble
// applied so band boundaries finger into each other instead of being ruler
// straight. falls back to stone outside the table.
block_id ravine_strata_at(const ravine_strata *s, const ravine_params *p,
                          int y, float wx, float wz);

#endif
