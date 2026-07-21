#ifndef RENDER_TONEMAP_PASS_H
#define RENDER_TONEMAP_PASS_H

#include "../gl.h"
#include "tonemap_programs.h"
#include "tonemap_params.h"
#include "tonemap_quad.h"
#include "tonemap_lut.h"

// the actual gl pass: sample the hdr scene, expose, tonemap, grade, optionally
// lut, encode to gamma, write ldr. all the uniform packing lives here so the
// top-level tonemap struct stays a thin orchestrator.

// push every uniform the grade shader needs from params + exposure multiplier.
// `prog` must already be glUseProgram'd by the caller.
void tonemap_pass_set_uniforms(glid prog, const tonemap_params *p,
                               float exposure);

// run the grade pass. binds the program, the scene texture on unit 0 and (if
// enabled) the lut texture on unit 1, sets uniforms, draws the fullscreen tri.
// the caller is responsible for binding the destination framebuffer + viewport
// before calling. does nothing if programs arent ok.
void tonemap_pass_run(const tonemap_programs *prog,
                      const tonemap_params *p,
                      const tonemap_quad *quad,
                      tonemap_lut *lut,
                      glid scene_tex,
                      float exposure);

#endif
