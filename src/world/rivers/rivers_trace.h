#ifndef WORLD_RIVERS_TRACE_H
#define WORLD_RIVERS_TRACE_H

#include "rivers_field.h"
#include "rivers_types.h"

// the trace pass turns the accumulation field into actual channels.
//
// a cell becomes a river the moment its accumulation crosses river_threshold.
// the cell where that first happens (no upstream neighbour was already a river)
// is a SOURCE — a spring head. from every cell over threshold we don't re-walk
// the whole basin; instead one sweep flags wet cells, and a separate downhill
// walk from each source records an ordered vertex path so the carver can cut a
// continuous bed with a sensible water-surface gradient.
//
// water surface along a channel is monotonic non-increasing: a river can't run
// uphill, and where it meets a lake it adopts the lake level. resolving that is
// the fiddly part and lives in rivers_trace_resolve_levels.

// flag every cell over threshold as RIVER (or SOURCE) in field->wet. lake cells
// already set by the fill pass are left alone. returns the river cell count.
int rivers_trace_mark(rivers_field *f, const rivers_params *p);

// walk downhill from a single source cell, appending vertices to *path (a
// darray of rivers_vertex). stops at a lake, the region edge, a sink, or after
// max_trace_steps. returns the number of vertices appended.
int rivers_trace_path(rivers_field *f, const rivers_params *p,
                      int src_x, int src_z, rivers_vertex **path);

// resolve water_y along every wet cell so it never runs uphill. flood-style
// relaxation seeded from lakes and the sea. call after marking. returns 1 if it
// changed anything.
int rivers_trace_resolve_levels(rivers_field *f, const rivers_params *p);

#endif
