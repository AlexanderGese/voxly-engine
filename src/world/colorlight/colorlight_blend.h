#ifndef WORLD_COLORLIGHT_BLEND_H
#define WORLD_COLORLIGHT_BLEND_H
#include <stdint.h>
#include "colorlight_rgb.h"
#include "colorlight_packed.h"
// final mixing. the mesher has a scalar sunlight level (0..15) and a packed rgb
// block-light word per voxel; it wants one rgb color. sunlight is white but gets
// a time-of-day tint applied here (warm at dawn/dusk, cold at noon-ish) so the
// world isn't flat gray when the sun's the only source. block light keeps its
// own hue and is combined with max(), not add, so a torch next to daylight
// doesn't blow out to white.
// the daytime sun tint. set once per frame from the day/night cycle, sampled by
// every blend call. defaults to plain white if never set.
void colorlight_blend_set_sun_tint(colorlight_rgb tint);
colorlight_rgb colorlight_blend_sun_tint(void);
// the core call the mesher makes per vertex/voxel. sun is the scalar 0..15
// sunlight, blk is the packed rgb block light. returns the lit color 0..255.
colorlight_rgb colorlight_blend(uint8_t sun, colorlight_packed blk);
// same but takes an ambient floor so caves never go fully black (matches the
// engine's habit of a tiny minimum light). amb is 0..255 per channel.
colorlight_rgb colorlight_blend_ambient(uint8_t sun, colorlight_packed blk, colorlight_rgb amb);
uint16_t colorlight_blend_to_565(colorlight_rgb c);
#endif
