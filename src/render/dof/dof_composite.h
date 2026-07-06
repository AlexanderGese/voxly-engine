#ifndef RENDER_DOF_COMPOSITE_H
#define RENDER_DOF_COMPOSITE_H

#include "../gl.h"
#include "dof_programs.h"
#include "dof_target.h"
#include "dof_quad.h"
#include "dof_params.h"

// the composite: take the sharp full-res scene plus the half-res near and far
// gather buffers and blend them into the final image. order matters and is the
// whole reason near/far are separate buffers:
//
// 1. start from sharp color
// 2. lerp toward the FAR blur by the far coc weight (far never covers sharp
// foreground, so this is a clean depth-keyed blend)
// 3. lerp toward the NEAR blur by the dilated near coverage on TOP of that,
// because a near out-of-focus object physically occludes whatever is
// behind it — it bleeds over the sharp midground.
//
// the gpu pass does this in one fullscreen draw. the cpu helpers expose the
// blend math so a test can pin the ordering without a gl context.

// run the composite into the currently-bound framebuffer (caller sets it; 0
// for the backbuffer). dst_w/dst_h reset the viewport to full res. no-op if
// the program is missing.
void dof_composite_run(const dof_programs *prog,
                       const dof_quad *quad,
                       const dof_target *sharp,
                       const dof_target *near_buf,
                       const dof_target *far_buf,
                       const dof_params *params,
                       int dst_w, int dst_h);

// cpu reference of the per-channel blend. given a sharp value and the near/far
// blurred values plus their coverage weights (0..1), returns the composited
// value. `strength` scales the whole effect toward `sharp`.
float dof_composite_blend(float sharp, float near_v, float far_v,
                          float near_w, float far_w, float strength);

// map a signed coc (texels) to a 0..1 blend weight for the far field. clamps
// and normalizes against max_coc. near cocs return 0 (this is the far weight).
float dof_composite_far_weight(float coc_texels, float max_coc);

// same for the near field, with the dilation baked in so the near blur starts
// bleeding a little before the geometry edge.
float dof_composite_near_weight(float coc_texels, float max_coc, float dilate);

#endif
