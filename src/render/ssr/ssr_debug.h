#ifndef RENDER_SSR_DEBUG_H
#define RENDER_SSR_DEBUG_H
#include "ssr_gbuffer.h"
#include "ssr_depth.h"
#include "ssr_settings.h"
#include "ssr_stats.h"
#include "../../math/vec3.h"
#include "../../math/vec4.h"
// the cpu reference driver. given a gbuffer's worth of cpu-side buffers it runs
// the full ssr pipeline per pixel — build ray, march, resolve — into an output
// reflection buffer, accumulating stats along the way. this is what the unit
// tests run to pin the gpu shader's behaviour, and what the debug overlay calls
// to sanity-check hit ratios.
//
// it is slow on purpose. it is not a frame path. see ssao_compute.c for the
// same disclaimer and the same coffee.
// per-pixel cpu inputs (everything the shader would sample from textures).
typedef struct {
    const vec3  *view_pos;   // w*h view-space positions
    const vec3  *view_nrm;   // w*h view-space normals
    const vec3  *color;      // w*h lit scene color (we reflect this)
    const float *roughness;  // w*h roughness, or NULL for the default material
    const float *metalness;  // w*h metalness, or NULL
    int w, h;
} ssrx_debug_inputs;
// run the whole pipeline for one pixel. returns the premultiplied reflection
// (rgb*weight, weight in .w). folds the march outcome into `stats` if non-null.
vec4 ssrx_debug_pixel(const ssrx_debug_inputs *in,
                      const ssrx_gbuffer *g, const ssrx_depthbuf *depth,
                      const ssrx_settings *s, ssrx_stats *stats,
                      int px, int py);
// fill an entire w*h reflection buffer (vec4 per pixel). resets `stats` first
// then accumulates across the frame. out must hold w*h vec4.
void ssrx_debug_buffer(const ssrx_debug_inputs *in,
                       const ssrx_gbuffer *g, const ssrx_depthbuf *depth,
                       const ssrx_settings *s, ssrx_stats *stats,
                       vec4 *out);
void ssrx_debug_composite(const vec3 *scene, const vec4 *refl,
                          vec3 *out, int w, int h);
#endif
