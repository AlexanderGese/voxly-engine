#ifndef ENTITY_NAVMESH_NAV_CELL_H
#define ENTITY_NAVMESH_NAV_CELL_H
#include "nav_types.h"
#include "../../world/world.h"
// the walkability primitive. decides whether an agent can stand on a given
// block column, and walks a column top-to-bottom finding every floor span.
// the builder calls these; nothing here touches the cell array.
// can an agent stand with its feet on top of block (wx,wy,wz)? that means:
// the floor block is solid, and the AGENT_HEIGHT blocks above it are clear.
// liquids don't count as floor (mobs sink) and don't count as clearance air.
int nav_cell_standable(world *w, int wx, int wy, int wz);
// is this block something an agent can pass its body through (air / non-solid
// and not a fluid we'd drown in)? used for the head-clearance check.
int nav_cell_passable(world *w, int wx, int wy, int wz);
// walk the column (wx, *, wz) over [y_lo, y_hi] and report each standable
// floor y into `out` (capped at `max`). returns how many we found. we scan
// bottom-up and skip the body span above a floor so a tall cave only yields
int nav_cell_spans(world *w, int wx, int wz, int y_lo, int y_hi,
                   int16_t *out, int max);
#endif
