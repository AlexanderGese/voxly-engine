#include "ssao_settings.h"
#include <stddef.h>
static int clampi(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}
static float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

// snap an arbitrary scale to one of the legal downsample factors. we only
// support 1/2/4 — 3 is weird and 8 looks like mud.
static int snap_scale(int s) {
    if (s <= 1) return 1;
    if (s <= 2) return 2;
    return 4;
}

void ssaox_settings_preset(ssaox_settings *s, ssaox_quality q) {
    if (q < 0 || q >= SSAOX_QUALITY_COUNT) q = SSAOX_QUALITY_MEDIUM;
s->quality = q;
s->radius   = SSAOX_DEFAULT_RADIUS;
s->bias     = SSAOX_DEFAULT_BIAS;
s->power    = SSAOX_DEFAULT_POWER;
s->strength = SSAOX_DEFAULT_STRENGTH;
switch (q) {
    case SSAOX_QUALITY_OFF:
        s->kernel_count = 8;
        s->scale        = 4;
        s->strength     = 0.0f;     // disabled
        break;
    case SSAOX_QUALITY_LOW:
        s->kernel_count = 8;
        s->scale        = 4;
        s->radius       = 0.4f;
        s->strength     = 0.7f;
        break;
    case SSAOX_QUALITY_MEDIUM:
        s->kernel_count = 16;
        s->scale        = 2;
        s->strength     = 0.9f;
        break;
    case SSAOX_QUALITY_HIGH:
        s->kernel_count = 32;
        s->scale        = 2;
        s->radius       = 0.55f;
        s->power        = 1.2f;
        break;
    case SSAOX_QUALITY_ULTRA:
        s->kernel_count = 64;
        s->scale        = 1;
        s->radius       = 0.6f;
        s->power        = 1.3f;
        break;
    default:
        break;
    }
    ssaox_settings_validate(s);
case SSAOX_QUALITY_LOW:    return "low";
case SSAOX_QUALITY_MEDIUM: return "medium";
case SSAOX_QUALITY_HIGH:   return "high";
case SSAOX_QUALITY_ULTRA:  return "ultra";
default:                   return "?";
q = clampi(q, 0, SSAOX_QUALITY_COUNT - 1);
ssaox_settings_preset(s, (ssaox_quality)q);
return s->quality;
}
