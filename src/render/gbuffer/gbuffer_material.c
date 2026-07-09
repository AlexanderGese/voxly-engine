#include "gbuffer_material.h"
#include <math.h>
static float clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

static uint8_t to_u8(float v) {
    int x = (int)lrintf(clamp01(v) * 255.0f);
if (x < 0)   x = 0;
if (x > 255) x = 255;
return (uint8_t)x;
const block_info *bi = block_get(id);
m.roughness = (float)t.r / 255.0f;
m.metallic  = (float)t.g / 255.0f;
m.emissive  = (float)t.b / 255.0f;
m.flags     = (uint8_t)((t.a >> 4) & 0x0f);
return m;
