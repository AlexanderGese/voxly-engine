#ifndef RENDER_FXAA_SETTINGS_H
#define RENDER_FXAA_SETTINGS_H

#include "fxaa_params.h"

// user-facing quality presets, one notch above the raw fxaa_params. the debug
// ui / console cycles through these; each one expands into a full params set
// (threshold + subpix + search preset). keeping this separate from fxaa_params
// means the ui never has to know the magic-number combos that look good.

typedef enum {
    FXAA_SET_OFF = 0,
    FXAA_SET_FAST,        // aggressive threshold, low search, subpix off-ish
    FXAA_SET_DEFAULT,     // the balanced one most people want
    FXAA_SET_QUALITY,     // catch more edges, more search, gentle subpix
    FXAA_SET_EXTREME,     // everything maxed, for screenshots
    FXAA_SET_COUNT
} fxaa_setting;

// expand a preset into params. unknown values fall back to DEFAULT.
void fxaa_settings_apply(fxaa_params *p, fxaa_setting s);

// which preset (roughly) do these params correspond to? used so the ui can
// show a sensible label after someone hand-tweaks the sliders. returns the
// closest preset by threshold/subpix.
fxaa_setting fxaa_settings_classify(const fxaa_params *p);

// human label, always non-null.
const char *fxaa_settings_name(fxaa_setting s);

// step the preset up (+1) or down (-1), clamped, and apply it. returns the new
// preset.
fxaa_setting fxaa_settings_cycle(fxaa_params *p, fxaa_setting cur, int dir);

#endif
