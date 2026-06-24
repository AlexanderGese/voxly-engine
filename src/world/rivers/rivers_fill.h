#ifndef WORLD_RIVERS_FILL_H
#define WORLD_RIVERS_FILL_H

#include "rivers_field.h"
#include "rivers_types.h"

// depression filling + lake detection, priority-flood style (barnes 2014).
//
// the core idea: start from the region boundary (water can always escape there)
// and grow inward, always expanding from the lowest cell on the frontier. when
// you step into a cell lower than the current "spill" height, you've found a
// pit — you raise it up to the spill height, and that raised amount is standing
// water. the heap keeps the frontier ordered so the spill height is correct
// without any iterate-til-stable nonsense.
//
// two things come out of this:
// field->filled  - the hydrologically-corrected surface, no closed pits left.
// the flow pass needs this so directions never dead-end.
// ponded basins  - cells the fill raised get marked RIVERS_LAKE in field->wet
// with their water_y set, as long as the basin is big enough
// and under fill_max_y.

// run the full fill. reads field->surface, writes field->filled, and marks lake
// cells into field->wet / field->water_y. returns the number of lake cells.
int rivers_fill_run(rivers_field *f, const rivers_params *p);

// just the depression fill, no lake marking. exposed mostly so the flow pass
// can be tested without dragging in the wet-marking logic.
void rivers_fill_depressions(rivers_field *f, const rivers_params *p);

#endif
