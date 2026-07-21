#ifndef RENDER_TONEMAP_PASS_H
#define RENDER_TONEMAP_PASS_H
#include "../gl.h"
#include "tonemap_programs.h"
#include "tonemap_params.h"
#include "tonemap_quad.h"
#include "tonemap_lut.h"
// the actual gl pass: sample the hdr scene, expose, tonemap, grade, optionally
// lut, encode to gamma, write ldr. all the uniform packing lives here so the
void tonemap_pass_set_uniforms(glid prog, const tonemap_params *p,
                               float exposure);
void tonemap_pass_run(const tonemap_programs *prog,
                      const tonemap_params *p,
                      const tonemap_quad *quad,
                      tonemap_lut *lut,
                      glid scene_tex,
                      float exposure);
#endif
