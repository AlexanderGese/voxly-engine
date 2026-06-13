#ifndef WORLD_BIOME_COLOR_H
#define WORLD_BIOME_COLOR_H

#include <stdint.h>
#include "biome_def.h"
#include "biome_climate.h"

// climate-driven tinting. grass/foliage/water colors shift with temperature
// and humidity, the way real grass browns out in dry heat and deepens in the
// wet. the biome def carries a base tint; we modulate it here so a forest on
// the dry edge looks subtly different from one in the rain.

// packed 0xRRGGBB grass tint for a climate. base is usually def->grass_tint.
uint32_t biome_color_grass(uint32_t base, const biome_climate *c);

// foliage (leaves) runs a touch darker/bluer than grass.
uint32_t biome_color_foliage(uint32_t base, const biome_climate *c);

// water tint, mostly a swamp/ocean thing. warmer water reads greener.
uint32_t biome_color_water(uint32_t base, const biome_climate *c);

// blend two packed colors, t in [0,1]. exposed because the blender and the
// edge fade both want it.
uint32_t biome_color_mix(uint32_t a, uint32_t b, float t);

#endif
