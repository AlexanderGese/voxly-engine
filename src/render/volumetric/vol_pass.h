#ifndef RENDER_VOLUMETRIC_PASS_H
#define RENDER_VOLUMETRIC_PASS_H
#include "vol_config.h"
#include "vol_params.h"
#include "vol_target.h"
#include "vol_quad.h"
#include "vol_programs.h"
#include "vol_blur.h"
#include "vol_dither.h"
#include "vol_medium.h"
#include "../../math/mat4.h"
#include "../../math/vec3.h"
typedef struct {
    volumetric_params   params;
    volumetric_target   target;
    volumetric_quad     quad;
    volumetric_programs progs;
    volumetric_blur     blur;
    volumetric_dither   dither;

    int  full_w, full_h;   // scene resolution we're sized for
    int  enabled;          // shaders ok AND params.enabled

    // per-frame inputs, cached so the debug overlay can read them back.
    vec3  to_sun;          // normalized direction toward the sun
    vec3  sun_color;       // light colour * intensity, fed to the march
    volumetric_medium_profile profile;  // last time-of-day profile

    unsigned frame;        // bumped each run; used to animate the dither
} volumetric_pass;
#endif
