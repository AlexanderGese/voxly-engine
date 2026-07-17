#include "ssr_debug.h"
#include "ssr_ray.h"
#include "ssr_march.h"
#include "ssr_resolve.h"
#include "ssr_material.h"
#include "ssr_jitter.h"

#include <stddef.h>

// the color sampler the resolve stage calls back into. we wrap the cpu color
// buffer + its dims so resolve can fetch the lit color at the hit uv. nearest
// fetch, clamped — matches the gpu sampler closely enough for the reference.
typedef struct {
    const vec3 *color;
    int w, h;
} color_ctx;

static int clampi(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

static vec3 sample_color_cb(void *user, vec2 uv) {
    const color_ctx *c = (const color_ctx *)user;
    int x = clampi((int)(uv.x * (float)c->w), 0, c->w - 1);
    int y = clampi((int)(uv.y * (float)c->h), 0, c->h - 1);
    return c->color[y * c->w + x];
}

// pull the material for a pixel: from the per-pixel buffers if present, else the
// global default. albedo comes from the lit color (good enough for metal f0).
static ssrx_material material_at(const ssrx_debug_inputs *in, int idx) {
    if (!in->roughness || !in->metalness) {
        ssrx_material m = ssrx_material_default();
        m.albedo = in->color[idx];
        return m;
    }
    return ssrx_material_unpack(in->roughness[idx], in->metalness[idx],
                               in->color[idx]);
}

vec4 ssrx_debug_pixel(const ssrx_debug_inputs *in,
                      const ssrx_gbuffer *g, const ssrx_depthbuf *depth,
                      const ssrx_settings *s, ssrx_stats *stats,
                      int px, int py) {
    int idx = py * in->w + px;

    // build the reflected ray for this surface fragment.
    ssrx_ray ray = ssrx_ray_make(in->view_pos[idx], in->view_nrm[idx]);

    // march it, with a per-pixel start jitter to break the banding the geometric
    // stride produces on flat reflectors.
    ssrx_march_params mp;
    ssrx_settings_to_march(s, &mp);
    ssrx_jitter jit;
    ssrx_jitter_default(&jit);
    mp.start_jitter = ssrx_jitter_offset(&jit, px, py);
    ssrx_march_result hit = ssrx_march_run(&ray, g, depth, &mp);

    if (stats) ssrx_stats_add(stats, &hit, ray.valid);

    // resolve into a premultiplied reflection.
    ssrx_resolve_params rp;
    ssrx_settings_to_resolve(s, &rp);
    ssrx_material mat = material_at(in, idx);

    color_ctx cc = { in->color, in->w, in->h };
    return ssrx_resolve_fragment(&hit, &ray, &mat, &rp, sample_color_cb, &cc);
}

void ssrx_debug_buffer(const ssrx_debug_inputs *in,
                       const ssrx_gbuffer *g, const ssrx_depthbuf *depth,
                       const ssrx_settings *s, ssrx_stats *stats,
                       vec4 *out) {
    if (stats) ssrx_stats_reset(stats);
    for (int y = 0; y < in->h; y++) {
        for (int x = 0; x < in->w; x++) {
            out[y * in->w + x] =
                ssrx_debug_pixel(in, g, depth, s, stats, x, y);
        }
    }
}

void ssrx_debug_composite(const vec3 *scene, const vec4 *refl,
                          vec3 *out, int w, int h) {
    for (int i = 0; i < w * h; i++) {
        vec3 base = scene[i];
        vec4 r    = refl[i];
        // premultiplied over: out = scene*(1-w) + refl.rgb (already *w).
        float inv = 1.0f - r.w;
        out[i] = vec3_new(base.x * inv + r.x,
                          base.y * inv + r.y,
                          base.z * inv + r.z);
    }
}
