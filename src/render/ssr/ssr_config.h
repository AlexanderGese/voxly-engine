#ifndef RENDER_SSR_CONFIG_H
#define RENDER_SSR_CONFIG_H

// tunables for the screen-space reflections subsystem.
// prefix is ssrx_ everywhere so this doesnt collide with anything in
// render/water/ (which does its own planar reflection via a second camera).
// ssr here is the cheap-and-cheerful kind: ray march the depth buffer in view
// space, no extra render of the scene. good enough for wet stone and metal.
//
// nothing here is law. step counts and thresholds get hand-tuned per scene.

#include "../gl.h"

// the reflection buffer runs at full_res / scale. reflections are blurry-ish
// and the resolve blends them under a roughness term anyway, so half res is
// nearly free and barely visible. set scale to 1 if you have gpu to spare.
#define SSRX_DEFAULT_SCALE      2

// linear march: how many fixed steps we walk along the reflected ray before
// giving up. more steps = longer reflections but more fill cost. 32 is the
// sweet spot for our render distance; 64 is for screenshots.
#define SSRX_MAX_STEPS          64
#define SSRX_DEFAULT_STEPS      32

// after the linear march finds the first step that went *behind* a surface we
// binary-refine the hit to kill the stair-stepping. this many bisections.
#define SSRX_MAX_REFINE         8
#define SSRX_DEFAULT_REFINE     5

// march stride in view-space units for the *first* step. the ray grows this
// geometrically (see SSRX_STRIDE_GROWTH) so near hits are precise and far ones
// are cheap. tuned against block-scale geometry (1 unit == 1 block).
#define SSRX_DEFAULT_STRIDE     0.25f
#define SSRX_STRIDE_GROWTH      1.06f

// a hit counts when the marched ray point is within this view-z band of the
// real scene depth. too tight and reflections get holes on grazing angles, too
// loose and they smear through thin geometry. fraction of stride, roughly.
#define SSRX_DEFAULT_THICKNESS  0.5f

// fresnel: reflectance at normal incidence. dielectric-ish baseline; the
// material pass can push it up for metals. schlick from here.
#define SSRX_DEFAULT_F0         0.04f
#define SSRX_FRESNEL_MIN        0.0f
#define SSRX_FRESNEL_MAX        1.0f

// edge fade: reflections that march off the side of the screen have no data,
// so we fade the reflection out as the hit uv approaches the border. this is
// the border width as a fraction of the screen (each side).
#define SSRX_DEFAULT_EDGE_FADE  0.12f

// distance fade: long reflections lose coherence (and are usually wrong), so
// we taper contribution as the march travels. fraction of max march length
// over which we ramp to zero, measured from the far end.
#define SSRX_DEFAULT_DIST_FADE  0.25f

// reflections off near-grazing rays (ray pointing back at the camera) are
// unstable; cull them below this dot(reflect, view) magnitude.
#define SSRX_BACKFACE_EPS       0.001f

// final blend amount 0..1, the master strength knob.
#define SSRX_DEFAULT_STRENGTH   0.8f

// roughness blur: the half-res reflection gets a confidence-weighted box blur
// whose radius scales with surface roughness (one sharp ray, fake the cone).
// this is the cap on that radius in (half-res) texels.
#define SSRX_BLUR_MAX_RADIUS    4

// texture units we expect to bind in the reflect pass.
#define SSRX_TEX_UNIT_DEPTH     0
#define SSRX_TEX_UNIT_NORMAL    1
#define SSRX_TEX_UNIT_COLOR     2   // the lit scene we sample reflections from
#define SSRX_TEX_UNIT_MATERIAL  3   // roughness/metalness, optional

// and in the resolve pass.
#define SSRX_TEX_UNIT_SCENE     0
#define SSRX_TEX_UNIT_REFLECT   1

// shader paths. these live alongside the rest under shaders/, copied there by
// the build. theyre optional at runtime — if missing the gpu path disables and
// the cpu reference march still works for tests.
#define SSRX_VERT_PATH          "shaders/post_passthrough.vert"
#define SSRX_FRAG_REFLECT_PATH  "shaders/ssrx_reflect.frag"
#define SSRX_FRAG_RESOLVE_PATH  "shaders/ssrx_resolve.frag"
#define SSRX_FRAG_BLUR_PATH     "shaders/ssrx_blur.frag"

#endif
