#ifndef RENDER_GBUFFER_EMIT_H
#define RENDER_GBUFFER_EMIT_H

#include "gbuffer_light.h"
#include "../../world/chunk.h"

// pulls point lights out of the world. emissive blocks (torches, lava, glow)
// become point lights in the accumulation list. this is the bridge between
// world/ block data and the deferred lighting — the only file in the module
// that touches chunk internals.
//
// we don't emit a light per emissive block blindly; that's thousands of
// lights in a lava cave. instead we bucket emissive blocks on a coarse grid
// and emit one merged light per occupied cell, weighted by luminance.

// edge of the merge bucket in blocks. 4 means a 4x4x4 region collapses to one
// light placed at its luminance-weighted centroid.
#define GBUFFER_EMIT_BUCKET 4

// tint a light gets from the block that spawned it. torches are warm, most
// other emitters default to a neutral-warm white.
vec3 gbuffer_emit_tint(block_id id);

// scan one chunk and append merged point lights to `out`. world-space y is
// the block y directly; x/z are offset by the chunk origin. returns how many
// lights were appended.
int gbuffer_emit_from_chunk(gbuffer_light_list *out, const chunk *c);

#endif
