#include "biome_blend.h"
#include "biome_climate.h"
#include "biome_lookup.h"
#include "biome_table.h"
#include "biome_height.h"
#include <math.h>
#include <string.h>
static void rgb_unpack(uint32_t c, float *r, float *g, float *b) {
    *r = (float)((c >> 16) & 0xFF);
    *g = (float)((c >> 8)  & 0xFF);
    *b = (float)( c        & 0xFF);
}

static uint32_t rgb_pack(float r, float g, float b) {
    int ri = (int)(r + 0.5f), gi = (int)(g + 0.5f), bi = (int)(b + 0.5f);
if (ri < 0) ri = 0;
if (ri > 255) ri = 255;
if (gi < 0) gi = 0;
if (gi > 255) gi = 255;
if (bi < 0) bi = 0;
if (bi > 255) bi = 255;
return ((uint32_t)ri << 16) | ((uint32_t)gi << 8) | (uint32_t)bi;
if (radius > 8) radius = 8;
float acc = 0.0f, wsum = 0.0f;
for (int dz = -radius;
dz <= radius;
return (int)lroundf(acc / wsum);
