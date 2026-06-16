#ifndef WORLD_FLUID_FLOW_DIRECTION_H
#define WORLD_FLUID_FLOW_DIRECTION_H

#include "fluid_cell.h"

// the visible flow direction of a fluid cell, derived from the level
// gradient to its neighbours. this is what pushes entities along and what
// the mesher uses to animate the surface.

typedef struct {
    float dx, dz;   // horizontal flow direction, not necessarily unit length
} voxl_fluid_flow;

// compute the flow vector for a cell. points toward lower-level neighbours
// (downhill). zero if the cell is flat / empty.
voxl_fluid_flow voxl_fluid_flow_at(const voxl_fluid_grid *g, int x, int y, int z);

// normalise a flow vector to unit length. zero stays zero.
voxl_fluid_flow voxl_fluid_flow_normalize(voxl_fluid_flow f);

// snap a flow vector to one of 8 compass directions, returns 0..7 (or -1 for
// no flow). handy for picking a flow sprite/rotation.
int voxl_fluid_flow_octant(voxl_fluid_flow f);

// the push force a flow imparts on an entity, scaled by strength.
voxl_fluid_flow voxl_fluid_flow_push(voxl_fluid_flow f, float strength);

#endif
