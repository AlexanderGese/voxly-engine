#ifndef WORLD_OREGEN_STAMP_H
#define WORLD_OREGEN_STAMP_H

#include "oregen.h"
#include "../chunk.h"

// optional chunk integration. the core driver stays world-coord and
// chunk-agnostic; this is the thin layer that actually writes cells into a
// single chunk during generation. keeping it separate means tests and tools
// can use the driver without dragging chunk.h in.

// default replace rule: ore only eats solid stone. air, water, bedrock,
// existing ore and anything decorative survives, so veins never float in
// caves or punch through the surface.
int oregen_stamp_default_replace(block_id existing);

// stamp every cell in buf that lands inside chunk c and replaces stone.
// cells outside the chunk footprint (veins seeded near the edge can reach
// into neighbors) are skipped here; the neighbor chunk re-rolls the same
// vein and stamps its own slice, so coverage stays seamless. returns the
// number of blocks actually written.
int oregen_stamp_chunk(chunk *c, const oregen_buf *buf);

// convenience: seed + grow + stamp for one chunk in a single call. buf is
// caller-owned scratch (so we dont malloc per chunk). surface_y is the
// chunk's tallest column. returns blocks written.
int oregen_apply_chunk(chunk *c, oregen_buf *buf, int surface_y,
                       uint32_t world_seed);

#endif
