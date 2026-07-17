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
#define SSRX_DEFAULT_SCALE      2
#define SSRX_MAX_STEPS          64
#define SSRX_DEFAULT_STEPS      32
#define SSRX_MAX_REFINE         8
#define SSRX_DEFAULT_REFINE     5
#define SSRX_DEFAULT_STRIDE     0.25f
#define SSRX_STRIDE_GROWTH      1.06f
#define SSRX_DEFAULT_THICKNESS  0.5f
#define SSRX_DEFAULT_F0         0.04f
#define SSRX_FRESNEL_MIN        0.0f
#define SSRX_FRESNEL_MAX        1.0f
#define SSRX_DEFAULT_EDGE_FADE  0.12f
#define SSRX_DEFAULT_DIST_FADE  0.25f
#define SSRX_BACKFACE_EPS       0.001f
#define SSRX_DEFAULT_STRENGTH   0.8f
#define SSRX_BLUR_MAX_RADIUS    4
#define SSRX_TEX_UNIT_DEPTH     0
#define SSRX_TEX_UNIT_NORMAL    1
#define SSRX_TEX_UNIT_COLOR     2   // the lit scene we sample reflections from
#define SSRX_TEX_UNIT_MATERIAL  3   // roughness/metalness, optional
#define SSRX_TEX_UNIT_SCENE     0
#define SSRX_TEX_UNIT_REFLECT   1
#define SSRX_VERT_PATH          "shaders/post_passthrough.vert"
#define SSRX_FRAG_REFLECT_PATH  "shaders/ssrx_reflect.frag"
#define SSRX_FRAG_RESOLVE_PATH  "shaders/ssrx_resolve.frag"
#define SSRX_FRAG_BLUR_PATH     "shaders/ssrx_blur.frag"
#endif
