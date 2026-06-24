#ifndef WORLD_RIVERS_OUTLET_H
#define WORLD_RIVERS_OUTLET_H

#include "rivers_field.h"
#include "rivers_types.h"

// a filled lake has a spill point: the lowest cell on its rim, the saddle where
// water would brim over and start a downstream river. priority-flood already
// raised the basin to exactly that height, so the saddle is wherever a lake cell
// sits next to a dry cell at (or just above) the water level.
//
// this pass finds those saddles and notches a shallow spillway through them so
// the overflow has somewhere to go instead of the lake being a sealed bathtub.
// without it lakes look fine but never feed the rivers below them, which reads
// wrong the moment you follow one downhill.

// find and carve outlet notches for every lake in the field. needs the wet plane
// (lakes marked) and water_y populated. lowers the rim cell at each saddle to
// water level and marks it RIVERS_RIVER so the carve pass cuts a channel there.
// returns the number of outlets carved. no-op if params->carve_outlet is 0.
int rivers_outlet_carve(rivers_field *f, const rivers_params *p);

#endif
