#ifndef WORLD_COLORLIGHT_PACKED_H
#define WORLD_COLORLIGHT_PACKED_H

#include <stdint.h>
#include "colorlight_rgb.h"

// how a single voxel's colored light is stored on disk and in the grid.
//
// the flood runs in levels 0..MAX_LIGHT (15), same resolution as the scalar
// path, so each channel needs 4 bits. three channels = 12 bits, which fits a
// uint16 with 4 spare. we pack r,g,b into the low 12 bits:
//
// bits  0..3  -> red level   0..15
// bits  4..7  -> green level 0..15
// bits  8..11 -> blue level  0..15
// bits 12..15 -> reserved (flags later, e.g. "do not propagate")
//
// note this is LEVELS not bytes. the 0..255 rgb form only exists at the edges
// (emitter table input, mesher output). keeping the grid at 4 bits/channel
// halves the memory vs storing bytes and matches the bfs step granularity.

typedef uint16_t colorlight_packed;

#define COLORLIGHT_CHAN_BITS   4
#define COLORLIGHT_CHAN_MASK   0x0Fu
#define COLORLIGHT_R_SHIFT     0
#define COLORLIGHT_G_SHIFT     4
#define COLORLIGHT_B_SHIFT     8
#define COLORLIGHT_FLAG_SHIFT  12

// per-channel level get/set on a packed word. levels clamp to 0..15.
uint8_t colorlight_packed_r(colorlight_packed p);
uint8_t colorlight_packed_g(colorlight_packed p);
uint8_t colorlight_packed_b(colorlight_packed p);
uint8_t colorlight_packed_chan(colorlight_packed p, int chan); // chan 0/1/2

colorlight_packed colorlight_packed_set_r(colorlight_packed p, uint8_t v);
colorlight_packed colorlight_packed_set_g(colorlight_packed p, uint8_t v);
colorlight_packed colorlight_packed_set_b(colorlight_packed p, uint8_t v);
colorlight_packed colorlight_packed_set_chan(colorlight_packed p, int chan, uint8_t v);

colorlight_packed colorlight_packed_make(uint8_t r, uint8_t g, uint8_t b);

// widen a packed level triple to a 0..255 rgb. levels are scaled by 17 so 15
// maps exactly to 255 (15*17 = 255), no fudge factor. this is the boundary
// where bfs levels become real color.
colorlight_rgb colorlight_packed_widen(colorlight_packed p);

// narrow a 0..255 rgb down to packed levels. rounds to nearest level. used to
// seed the grid from the emitter table.
colorlight_packed colorlight_packed_narrow(colorlight_rgb c);

// brightest channel level, 0..15. floods early-out on this.
uint8_t colorlight_packed_peak(colorlight_packed p);

#endif
