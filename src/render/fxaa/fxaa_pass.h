#ifndef RENDER_FXAA_PASS_H
#define RENDER_FXAA_PASS_H

#include "../gl.h"
#include "fxaa_programs.h"
#include "fxaa_params.h"
#include "fxaa_quad.h"
#include "fxaa_target.h"

// the two gpu stages. the manager owns the resources and just calls these in
// order. kept thin so the wiring (texture binds, uniforms, viewport) is all in
// one readable place.

// prepass: read the ldr scene color, compute luma, write rgba8 with the luma
// in alpha into `dst`. this is the only stage that reads the raw scene.
void fxaa_pass_prepass(const fxaa_programs *prog,
                       const fxaa_quad *quad,
                       const fxaa_target *dst,
                       glid scene_tex);

// main: read the prepass target, run the edge filter, write the antialiased
// result into the framebuffer `out_fbo` (0 = backbuffer) at out_w x out_h.
// `derived` is fxaa_params_derive output; `rcp` is the 1/texel-size the
// shader needs for neighbour offsets.
void fxaa_pass_main(const fxaa_programs *prog,
                    const fxaa_quad *quad,
                    const fxaa_target *src,
                    const fxaa_params *params,
                    const float derived[4],
                    int quality_steps,
                    glid out_fbo, int out_w, int out_h);

#endif
