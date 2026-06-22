#ifndef WORLD_RAVINE_DETAIL_H
#define WORLD_RAVINE_DETAIL_H

#include "ravine_field.h"
#include "ravine_strata.h"
#include "../chunk.h"

// the post-carve detailer. a clean falloff carve leaves walls that are a touch
// too tidy — a smooth ramp of strata, no character. this pass roughens them
// using the one band field the silhouette ignores: hardness. hard bands jut a
// block out into the gap (resistant rock weathers slower), soft bands get an
// extra block shaved back (it crumbles), and loose debris from the soft bands
// settles as a thin scree of cobble on the channel floor below. all keyed on
// world coords + noise so the two sides of a ravine roughen independently but
// deterministically.
//
// this runs after ravine_carve_apply on the same chunk. it only ever adds a
// block into existing air (the jut) or turns a wall block to air (the shave) or
// drops scree onto a floor — it never deepens the cut, so it cant breach the
// bedrock guard the carve already respected.

typedef struct {
    int juts;        // hard-rock blocks pushed out into the gap
    int shaves;      // soft-rock blocks crumbled back to air
    int scree;       // debris blocks dropped on the floor
} ravine_detail_stats;

// run the detailer over the chunk. needs the same field + strata the carve used.
ravine_detail_stats ravine_detail_apply(chunk *c, const ravine_field *f,
                                        const ravine_strata *strata,
                                        const ravine_params *p);

#endif
