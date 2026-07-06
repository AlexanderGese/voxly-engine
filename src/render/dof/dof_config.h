#ifndef RENDER_DOF_CONFIG_H
#define RENDER_DOF_CONFIG_H
// tunables for the depth-of-field subsystem. thin-lens bokeh, the kind of
// dof you get from a real camera rather than the cheap "blur everything far
// away" trick. prefix is dofx_ so we dont collide with anything else if some
// older blur pass ever shows up under the dof_ name.
//
// nothing here is sacred. focus distance gets driven by the autofocus or the
// player's look-at every frame; the rest is hand-tuned per scene.
#include "../gl.h"
// bokeh gather kernel. these are disk sample offsets in normalized texels,
// laid out as a sunflower/golden-spiral so the ring pattern doesnt show even
// at low counts. 32 taps is the comfortable middle; 16 is grainy but cheap,
// 64 is gorgeous and melts the potato target.
#define DOFX_KERNEL_MAX         64
#define DOFX_KERNEL_DEFAULT     32
// the coc + gather buffers run at half res by default. dof is low frequency
// (its literally blur) so nobody notices, and it quarters the fill cost. set
// scale to 1 if you have gpu to burn, 4 if youre desperate.
#define DOFX_DEFAULT_SCALE      2
// thin-lens parameters. focal length and aperture are in the same world unit
// as the scene (blocks). f-stop = focal_len / aperture_diameter, so a small
// f-stop is a wide aperture is a shallow depth of field.
#define DOFX_DEFAULT_FOCUS_DIST 12.0f   // where the plane of focus sits
#define DOFX_DEFAULT_FOCAL_LEN  0.05f   // ~50mm-ish in block units
#define DOFX_DEFAULT_FSTOP      2.8f    // aperture, smaller = blurrier
#define DOFX_DEFAULT_SENSOR     0.036f  // 35mm sensor width-ish
// circle of confusion is clamped to this radius (in half-res texels) so a tap
#define DOFX_MAX_COC_TEXELS     16.0f
#define DOFX_FOCUS_EPSILON      0.5f
#define DOFX_NEAR_DILATE        2
#define DOFX_DEFAULT_STRENGTH   1.0f
#define DOFX_TEX_UNIT_COLOR     0
#define DOFX_TEX_UNIT_DEPTH     1
#define DOFX_TEX_UNIT_COC       2
#define DOFX_TEX_UNIT_NEAR      3
#define DOFX_TEX_UNIT_FAR       4
#define DOFX_VERT_PATH          "shaders/post_passthrough.vert"
#define DOFX_FRAG_COC_PATH      "shaders/dofx_coc.frag"
#define DOFX_FRAG_GATHER_PATH   "shaders/dofx_gather.frag"
#define DOFX_FRAG_COMPOSITE_PATH "shaders/dofx_composite.frag"
#endif
