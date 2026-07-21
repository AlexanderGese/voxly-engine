#ifndef RENDER_VOLUMETRIC_CONFIG_H
#define RENDER_VOLUMETRIC_CONFIG_H

// tunables for the volumetric light / god-ray subsystem.
// we raymarch the camera frustum against the sun's shadow map, accumulate
// in-scattering with a henyey-greenstein phase, dither the start offset to
// trade banding for noise, then blur + composite additively over the scene.
//
// none of these are sacred. the step count / scattering coeff get hand-tuned
// per time-of-day anyway; the defaults are "looks fine at noon".

#include "../gl.h"

// number of ray steps along the view ray. more steps = smoother shafts and
// fewer banding artifacts, but it's the whole cost of the pass. 64 is the
// honest minimum once you've got dithering hiding the steps; 128 is pretty.
// keep it a multiple of 8 so the (optional) early-out unroll stays tidy.
#define VOL_STEPS_MAX           192
#define VOL_STEPS_DEFAULT        64

// we march at reduced resolution because the result is low frequency and the
// blur smears the rest. half res is the sweet spot. 1 if you have fill to
// burn, 4 if you're on a toaster (the shafts get chunky).
#define VOL_DEFAULT_SCALE         2

// henyey-greenstein anisotropy g in (-1,1). positive = forward scattering
// (bright halo around the sun), which is what air actually does. 0 = uniform.
// 0.999 = laser. we clamp to this in the setter so the phase fn doesn't blow.
#define VOL_G_DEFAULT          0.76f
#define VOL_G_MAX              0.95f

// scattering / extinction coefficients (per world unit). scattering is how
// much light the medium kicks toward the eye; extinction is how fast the ray
// (and the accumulated light) gets absorbed. keep extinction >= scattering
// or you create energy, which the validator in vol_phase will whine about.
#define VOL_SCATTER_DEFAULT    0.030f
#define VOL_EXTINCT_DEFAULT    0.040f

// overall intensity knob folded in at composite time. the physical numbers
// above give a subtle effect; this is the "make it pop" multiplier.
#define VOL_INTENSITY_DEFAULT  1.4f

// dither tile. a small bayer matrix tiled across the screen jitters each ray's
// start so the step banding turns into high-freq noise the blur eats. 4x4 is
// enough; 8x8 is smoother but the pattern is already invisible post-blur.
#define VOL_DITHER_DIM            4
#define VOL_DITHER_TEXELS        (VOL_DITHER_DIM * VOL_DITHER_DIM)

// separable gaussian blur radius per side, in texels (so 2 -> a 5-tap pass).
// applied horizontally then vertically. bilateral weighting on depth keeps
// the shafts from leaking across silhouettes.
#define VOL_BLUR_RADIUS           2
#define VOL_BLUR_DEPTH_SIGMA   0.0025f   // ndc-depth falloff for the bilateral term

// how far down the ray we bother marching, in world units. beyond this the
// transmittance has usually decayed to nothing anyway. clamped against the
// camera far plane at runtime.
#define VOL_MAX_DISTANCE       180.0f

// texture units the march shader expects bound.
#define VOL_TEX_UNIT_DEPTH        0
#define VOL_TEX_UNIT_SHADOW       1
#define VOL_TEX_UNIT_DITHER       2
#define VOL_TEX_UNIT_SCATTER      0   // for the blur + composite passes
#define VOL_TEX_UNIT_SCENE        1   // composite: the lit scene color

// shader paths, copied next to the others under shaders/ by the build. as
// with ssao, if the .frag's are missing at runtime we disable the gpu path
// and the cpu reference march still runs for tests.
#define VOL_VERT_PATH          "shaders/vol_passthrough.vert"
#define VOL_FRAG_MARCH_PATH    "shaders/volumetric_march.frag"
#define VOL_FRAG_BLUR_PATH     "shaders/volumetric_blur.frag"
#define VOL_FRAG_COMP_PATH     "shaders/volumetric_composite.frag"

#endif
