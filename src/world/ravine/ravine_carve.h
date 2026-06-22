#ifndef WORLD_RAVINE_CARVE_H
#define WORLD_RAVINE_CARVE_H

#include "ravine_field.h"
#include "ravine_strata.h"
#include "../chunk.h"

// the bridge from the resolved field to an actual chunk. everything upstream is
// pure cell maths; this is where we finally touch blocks. for each column in the
// chunk footprint we clear from the old surface down to the resolved floor, then
// re-skin the freshly exposed wall faces with the strata block for their world
// height. floors that sit below sea level refill with water up to the waterline.

typedef struct {
    int air_set;        // blocks turned to air
    int water_set;      // blocks turned to water
    int strata_faced;   // wall blocks re-skinned with a strata block
    int skipped_bedrock;
} ravine_carve_stats;

// stamp the field into the chunk in place. only carves diggable solids — wont
// eat ores, water, or player builds, and never bedrock. strata is the pre-rolled
// band table for the ravine that owns this region.
ravine_carve_stats ravine_carve_apply(chunk *c, const ravine_field *f,
                                      const ravine_strata *strata,
                                      const ravine_params *p);

// true for blocks the carver is allowed to dig through.
int ravine_carve_is_diggable(block_id id);

#endif
