#include "vol_raymarch.h"
#include "vol_config.h"

#include <math.h>
#include <stddef.h>

// front-to-back accumulation. classic participating-media integration:
// L = sum_i  T_i * sigma_s * phase * sun_visible_i * sun_color * dx
// T_{i+1} = T_i * exp(-sigma_t * dx)
// nothing fancy — but it matches the .frag exactly, which is the point.

vec3 volumetric_world_from_depth(mat4 inv_view_proj,
                                 float ndc_x, float ndc_y, float depth01) {
    // depth buffer is [0,1]; ndc z is [-1,1]. unproject the clip-space point.
    vec3 ndc = vec3_new(ndc_x, ndc_y, depth01 * 2.0f - 1.0f);
    // mat4_mul_vec3 assumes w=1 on input but we still need the perspective
    // divide on the way out. do it the long way so we get the divide right.
    float x = ndc.x, y = ndc.y, z = ndc.z;
    const float (*m)[4] = (const float (*)[4])inv_view_proj.m;
    float ox = m[0][0]*x + m[1][0]*y + m[2][0]*z + m[3][0];
    float oy = m[0][1]*x + m[1][1]*y + m[2][1]*z + m[3][1];
    float oz = m[0][2]*x + m[1][2]*y + m[2][2]*z + m[3][2];
    float ow = m[0][3]*x + m[1][3]*y + m[2][3]*z + m[3][3];
    if (fabsf(ow) < 1e-8f) ow = (ow < 0.0f) ? -1e-8f : 1e-8f;
    float inv = 1.0f / ow;
    return vec3_new(ox * inv, oy * inv, oz * inv);
}

vec3 volumetric_march_ray(const volumetric_march_ctx *ctx,
                          vec3 origin, vec3 target,
                          int px, int py,
                          float *transmittance_out) {
    vec3 accum = VEC3_ZERO;
    float transmittance = 1.0f;

    int steps = ctx->steps;
    if (steps < 1) steps = 1;
    if (steps > VOL_STEPS_MAX) steps = VOL_STEPS_MAX;

    // ray from eye to the surface, clamped to the configured max distance.
    vec3 delta = vec3_sub(target, origin);
    float dist = vec3_length(delta);
    if (dist < 1e-4f) { if (transmittance_out) *transmittance_out = 1.0f; return accum; }
    if (dist > ctx->max_dist) dist = ctx->max_dist;

    vec3 dir = vec3_scale(delta, 1.0f / vec3_length(delta));
    float dx = dist / (float)steps;

    // dither the start so neighbouring rays don't band in lock-step. the offset
    // is a fraction of one step; transmittance for the skipped sliver is folded
    // into the first step's weight implicitly (it's tiny).
    float offset = 0.5f;
    if (ctx->dither) offset = volumetric_dither_at(ctx->dither, px, py);

    // phase is constant along a straight ray (single directional light), so
    // hoist it out of the loop. cos between the *view* dir and the to-sun dir.
    float cos_vl = volumetric_cos_between(dir, ctx->to_sun);
    float phase = volumetric_phase_hg(ctx->medium.g, cos_vl);

    float sigma_s = ctx->medium.scatter;
    float sigma_t = ctx->medium.extinct;
    float step_t = expf(-sigma_t * dx);   // per-step transmittance, constant

    for (int i = 0; i < steps; i++) {
        // sample at the dithered centre of step i
        float t = (((float)i + offset) ) * dx;
        if (t > dist) break;
        vec3 p = vec3_add(origin, vec3_scale(dir, t));

        int lit = 1;
        if (ctx->shadow) lit = ctx->shadow(ctx->shadow_user, p);

        if (lit) {
            // in-scattered contribution at this slab, attenuated by how much
            // light has already been absorbed getting here.
            float w = transmittance * sigma_s * phase * dx;
            accum = vec3_add(accum, vec3_scale(ctx->sun_color, w));
        }

        // advance transmittance through the slab regardless of lit/shadow —
        // the medium absorbs either way.
        transmittance *= step_t;

        // negligible-light early out. once we've absorbed essentially
        // everything there's no point marching the tail.
        if (transmittance < 0.003f) break;
    }

    if (transmittance_out) *transmittance_out = transmittance;
    return accum;
}
