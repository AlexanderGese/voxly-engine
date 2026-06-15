#ifndef WORLD_FARMING_H
#define WORLD_FARMING_H

// umbrella include for the farming subsystem. pull this and you get crops,
// farmland, hydration, growth, planting, harvest and the field manager that
// drives them. the rest of the engine only really needs farming_field.h, but
// this saves a stack of includes at the call site.
//
// quick tour of the flow:
// till   -> farming_field_till      (dirt becomes a tracked farmland tile)
// plant  -> farming_field_plant     (seed becomes a stage-0 crop)
// tick   -> farming_field_update    (hydration + growth + stems + decay)
// reap   -> farming_field_harvest   (mature crop -> yield, tile freed up)

#include "farming_types.h"
#include "farming_rng.h"
#include "farming_block.h"
#include "farming_def.h"
#include "farming_query.h"
#include "farming_hydration.h"
#include "farming_tile.h"
#include "farming_growth.h"
#include "farming_plant.h"
#include "farming_harvest.h"
#include "farming_stem.h"
#include "farming_fertilizer.h"
#include "farming_field.h"
#include "farming_serialize.h"
#include "farming_debug.h"

#endif
