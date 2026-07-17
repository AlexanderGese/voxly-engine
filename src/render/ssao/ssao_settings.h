#ifndef RENDER_SSAO_SETTINGS_H
#define RENDER_SSAO_SETTINGS_H

#include "ssao_config.h"

// user-facing quality knobs for ssao, the kind of thing that hangs off a
// settings menu. these map onto the raw ssaox_pass fields but with safe
// ranges and a few canned presets so the options screen doesnt have to know
// the gory details.

typedef enum {
    SSAOX_QUALITY_OFF = 0,
    SSAOX_QUALITY_LOW,
    SSAOX_QUALITY_MEDIUM,
    SSAOX_QUALITY_HIGH,
    SSAOX_QUALITY_ULTRA,
    SSAOX_QUALITY_COUNT
} ssaox_quality;

typedef struct {
    ssaox_quality quality;
    int   kernel_count;   // sample count
    int   scale;          // downsample factor (1,2,4)
    float radius;
    float bias;
    float power;
    float strength;       // final blend 0..1
} ssaox_settings;

// fill `s` with the canned preset for a quality level. OFF gives strength 0.
void  ssaox_settings_preset(ssaox_settings *s, ssaox_quality q);

// clamp every field into its legal range. returns 1 if anything was changed.
int   ssaox_settings_validate(ssaox_settings *s);

// human readable name for a quality level, for the options menu.
const char *ssaox_quality_name(ssaox_quality q);

// is ssao effectively a no-op for these settings? (off, or zero strength)
int   ssaox_settings_is_noop(const ssaox_settings *s);

// step the quality up or down one notch, clamped. dir > 0 = up. returns the
// new quality and rewrites `s` to that preset.
ssaox_quality ssaox_settings_cycle(ssaox_settings *s, int dir);

#endif
