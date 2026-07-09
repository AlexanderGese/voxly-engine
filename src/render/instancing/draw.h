#ifndef RENDER_INSTANCING_DRAW_H
#define RENDER_INSTANCING_DRAW_H

// the submission layer. takes a built batch + the mesh registry and issues the
// instanced draw calls. one group -> one mesh -> upload its record slice and
// fire glDrawArraysInstanced. groups bigger than INSTANCING_MAX_PER_BATCH are
// split into several calls so we never hand the driver a silly-large instance
// vbo in one shot.
//
// the caller owns gl state: shader must be bound, the per-instance attrib
// layout (loc 3..8) must match the shader, texture/blend set up. we only touch
// vao/vbo binds and the draw itself.

#include "batch.h"
#include "registry.h"
#include "instancing_stats.h"

// submit every group. for each, uploads the slice into the mesh's instance vbo
// and draws. base_tris per mesh is derived from its base_count (/3). stats are
// updated with draw-call and triangle counts.
void instancing_draw_batch(const instancing_batch *batch,
                           instancing_registry *reg,
                           instancing_stats *stats);

// draw a single group by index. exposed mostly for the multi-pass case where a
// caller wants to interleave groups with their own state changes.
void instancing_draw_group(const instancing_batch *batch, int group_index,
                           instancing_registry *reg,
                           instancing_stats *stats);

#endif
