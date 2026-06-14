#ifndef WORLD_COLORLIGHT_EMITTER_H
#define WORLD_COLORLIGHT_EMITTER_H
#include "../block.h"
#include "colorlight_rgb.h"
#include "colorlight_packed.h"
// which blocks emit colored light and what color. the scalar block_info already
typedef struct {
    block_id        block;
    colorlight_rgb  tint;     // hue at full strength, 0..255 per channel
    uint8_t         level;    // emission strength 0..MAX_LIGHT (overrides info)
} colorlight_emitter;
colorlight_rgb colorlight_emitter_color(block_id id);
colorlight_packed colorlight_emitter_seed(block_id id);
int colorlight_emitter_is(block_id id);
void colorlight_emitter_attenuation(block_id id, uint8_t out_rgb_cost[3]);
#endif
