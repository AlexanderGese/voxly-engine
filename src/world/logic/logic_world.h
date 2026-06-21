#ifndef WORLD_LOGIC_WORLD_H
#define WORLD_LOGIC_WORLD_H

#include "logic_net.h"
#include "../world.h"

// glue between the abstract logic net and the real block world. the net is
// deliberately world-agnostic (it just owns cells); this layer scans logic
// blocks out of chunks into the net, mirrors player edits, and reports lamp
// state back so the renderer/lighting can light them up.
//
// kept thin on purpose: no rendering, no save format. it only translates
// between block_id grids and logic_cell grids.

// scan a chunk's blocks and register every LOGIC_BLOCK_* it contains with the
// net. idempotent: re-scanning updates existing cells rather than duplicating.
// returns the number of logic cells found in the chunk.
int logic_world_scan_chunk(logic_net *net, const chunk *c);

// scan everything currently loaded. call once after worldgen, or after a bulk
// load. returns total cells registered.
int logic_world_scan_all(logic_net *net, world *w);

// mirror a single world edit into the net: a block was placed or cleared at a
// world coord. resolves the new block id from the world and adds/updates/drops
// the matching cell, then wakes the area.
void logic_world_on_block_changed(logic_net *net, world *w, int wx, int wy, int wz);

// is the lamp at this coord currently lit? false for non-lamp coords. the
// renderer uses this to pick the lit vs unlit tile and to inject block light.
int logic_world_lamp_lit(const logic_net *net, int wx, int wy, int wz);

// the effective block-light contribution of logic at a coord (lit lamps glow,
// lit torches glow a little). 0 if nothing logic-y is lit there. lets the
// lighting pass fold logic emitters into its flood.
uint8_t logic_world_light_at(const logic_net *net, int wx, int wy, int wz);

#endif
