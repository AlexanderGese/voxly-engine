#ifndef WORLD_EROSION_THERMAL_H
#define WORLD_EROSION_THERMAL_H

#include "erosion_types.h"
#include "erosion_field.h"

// thermal / talus erosion: the dry counterpart to the droplet pass. wherever
// the height difference between a cell and a downhill neighbour exceeds the
// talus angle, material slumps from the high cell to the low ones until the
// slope is back under the angle of repose. this is what turns the sharp
// canyons the water carves into believable scree slopes.

// one relaxation sweep over the whole field. moves material into a scratch
// delta buffer then applies it, so the sweep order doesnt bias the result.
// returns the total height moved this sweep (0 once everything is stable).
float erosion_thermal_sweep(erosion_field *f, const erosion_params *p);

// run p->thermal_iters sweeps. accumulates moved material into stats if given.
void erosion_thermal_pass(erosion_field *f, const erosion_params *p,
                          erosion_stats *st);

#endif
