#ifndef RENDER_DOF_GATHER_H
#define RENDER_DOF_GATHER_H
#include "../gl.h"
#include "dof_kernel.h"
#include "dof_coc.h"
#include "dof_target.h"
#include "dof_programs.h"
#include "dof_quad.h"
// the bokeh gather. for every output texel we walk the kernel, sampling the
// source color at offset*coc, and accumulate. two passes: near and far. they
// differ in which fragments they let contribute —
// far  : a tap contributes only if ITS coc is >= the centre coc (so a sharp
// foreground object never smears into the blurry background).
// near : every tap with a near coc contributes regardless of the centre,
// which is what lets near blur bleed over sharp midground.
//
// the gpu path is a fullscreen draw per pass; the cpu path here is a real,
// runnable reference used by tests and by the no-shader fallback. it's slow
// and that's fine, it's a reference.
// a tiny cpu image so the reference gather has something to chew. rgba float,
// row-major, owned by the caller.
typedef struct {
    float *px;   // w*h*4 floats, or NULL
    int    w, h;
} dof_image;
// --- gpu path ---------------------------------------------------------------
// run one gather pass into `dst`. `near_pass` picks the contribution rule
// above. binds color+coc, sets the per-pass uniforms and draws the quad.
// no-op if the program is missing.
void dof_gather_pass(const dof_programs *prog,
                     const dof_quad *quad,
                     const dof_target *color,
                     const dof_target *coc,
                     dof_target *dst,
                     int near_pass,
                     float texel_w, float texel_h);
// run both near and far passes back to back into their targets.
void dof_gather_run(const dof_programs *prog,
                    const dof_quad *quad,
                    const dof_target *color,
                    const dof_target *coc,
                    dof_target *near_dst,
                    dof_target *far_dst);
// --- cpu reference ----------------------------------------------------------
// gather a single texel (x,y) of `src` using `kernel` scaled by `coc_texels`
// (signed; sign selects near vs far rule). writes the resolved rgba into out4.
// out-of-bounds taps clamp to edge. returns the total weight used.
float dof_gather_texel(const dof_image *src, const dof_kernel *kernel,
                       int x, int y, float coc_texels, float *out4);
// gather the whole image with a flat (constant) coc. mostly a test harness:
// proves the kernel produces a symmetric disc blur. dst must be allocated to
int   dof_gather_image_flat(const dof_image *src, const dof_kernel *kernel,
                            float coc_texels, dof_image *dst);
#endif
