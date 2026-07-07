#ifndef RENDER_DOF_PASS_H
#define RENDER_DOF_PASS_H

#include "../gl.h"
#include "dof_programs.h"
#include "dof_target.h"
#include "dof_quad.h"
#include "dof_coc.h"

// the coc generation pass plus the half-res downsample of the scene color.
// these are the "front" of the chain: everything after them (gather, composite)
// works off the buffers these fill.
//
// coc pass    : reads the depth texture, reconstructs view-space distance,
// evaluates the thin-lens coc and writes signed coc into a
// single-channel-ish target (we stash it in .r, leave gba for
// debug overlays).
// prefilter   : downsamples the full-res hdr scene into the half-res color
// target the gather samples from. doing the gather at half res
// is the big perf win and the prefilter is where the res drop
// happens.

// fill `coc_dst` from the scene depth texture using the lens. depth_tex is the
// engine's depth attachment. near/far planes let the shader linearize depth.
void dof_pass_coc(const dof_programs *prog,
                  const dof_quad *quad,
                  glid depth_tex,
                  dof_target *coc_dst,
                  const dof_lens *lens,
                  float znear, float zfar,
                  float texel_scale);

// downsample full-res `scene_tex` into the half-res `color_dst`. uses a simple
// box reduction (the hardware linear filter does most of the work). reuses the
// gather program's color sampler binding for convenience.
void dof_pass_prefilter(const dof_programs *prog,
                        const dof_quad *quad,
                        glid scene_tex,
                        dof_target *color_dst);

#endif
