#ifndef WORLD_COLORLIGHT_EMITTER_H
#define WORLD_COLORLIGHT_EMITTER_H

#include "../block.h"
#include "colorlight_rgb.h"
#include "colorlight_packed.h"

// which blocks emit colored light and what color. the scalar block_info already
// carries emits_light + luminance (0..15); this table adds the hue. for blocks
// not in the table we fall back to a warm-white derived from block_info so the
// world still works if someone adds an emitter and forgets to tint it here.
//
// also holds per-block attenuation: how much a channel loses crossing a
// translucent block (water tints things blue-green, stained glass would filter
// a channel out). opaque blocks just stop the flood, they never get here.

typedef struct {
    block_id        block;
    colorlight_rgb  tint;     // hue at full strength, 0..255 per channel
    uint8_t         level;    // emission strength 0..MAX_LIGHT (overrides info)
} colorlight_emitter;

// rgb a torch-ish block of this id should radiate. returns black for
// non-emitters. combines the table tint with the block's luminance so a dim
// torch is a dim orange, not full orange at level 14.
colorlight_rgb colorlight_emitter_color(block_id id);

// packed starting levels to seed the flood with. == narrow(emitter_color).
colorlight_packed colorlight_emitter_seed(block_id id);

// is this block an rgb emitter at all? cheap gate for the chunk scan.
int colorlight_emitter_is(block_id id);

// per-channel extra falloff (0..15) a channel eats passing THROUGH this block,
// on top of the normal -1 per step. 0 for air/glass. water bleeds red/green
// faster than blue, so dives go teal. returns the three channel costs.
void colorlight_emitter_attenuation(block_id id, uint8_t out_rgb_cost[3]);

#endif
