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

// the whole volumetric light pass, wired together. owns the targets, the
// dither texture, the blur stage, the quad and the programs. you feed it the
// scene's depth + the sun's shadow map each frame; it marches, blurs, and
// composites the god rays additively over the lit scene.
//
// lifecycle mirrors the ssao pass: init at a resolution, resize on window
// changes, run() per frame. if the shaders didn't load it stays "disabled" and
// run() is a cheap no-op (the cpu reference march in vol_raymarch still works
// for headless tests).

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

// init at scene resolution. loads shaders, builds dither + blur kernel, sizes
// targets. returns 1 if the gpu path is live, 0 if it fell back to disabled
// (still safe to call run / resize / destroy).
int  volumetric_pass_init(volumetric_pass *p, int full_w, int full_h);

void volumetric_pass_destroy(volumetric_pass *p);

void volumetric_pass_resize(volumetric_pass *p, int full_w, int full_h);

// update the per-frame sun state. `to_sun` need not be normalized. this also
// refreshes the time-of-day medium profile and folds the profile into params.
void volumetric_pass_set_sun(volumetric_pass *p, vec3 to_sun, vec3 sun_color);

// run the full pipeline. inputs:
// depth_tex   — scene depth, sampled to reconstruct world pos + bilateral
// shadow_tex  — the sun's depth/shadow map
// inv_vp      — inverse view-projection, for the world reconstruction
// light_vp    — the sun's view-projection, for the shadow lookup
// scene_tex   — the lit scene colour the composite adds onto
// out_fbo     — destination framebuffer (0 = default) and its dimensions
// returns 1 if it drew, 0 if disabled. leaves out_fbo bound at full viewport.
int  volumetric_pass_run(volumetric_pass *p,
                         glid depth_tex, glid shadow_tex,
                         mat4 inv_vp, mat4 light_vp,
                         glid scene_tex,
                         glid out_fbo);

// whether the pass will actually do anything this frame.
int  volumetric_pass_active(const volumetric_pass *p);

#endif
