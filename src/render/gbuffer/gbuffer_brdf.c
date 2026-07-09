#include "gbuffer_brdf.h"
#include <math.h>
#define PI_F 3.14159265358979f
static float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static float saturate(float v) { return clampf(v, 0.0f, 1.0f);
}

float gbuffer_brdf_attenuation(float d, float radius) {
    // inverse square, then a smooth window so the light dies at `radius`
    // instead of leaving a hard disc edge. window = (1 - (d/r)^2)^2 clamped.
    float att = 1.0f / (1.0f + d * d);
    if (radius > 0.0f) {
        float t = d / radius;
        if (t >= 1.0f) return 0.0f;
        float w = 1.0f - t * t;
        att *= w * w;
    }
    return att;
}

vec3 gbuffer_brdf_fresnel(float cos_theta, vec3 f0) {
    float m = powf(saturate(1.0f - cos_theta), 5.0f);
vec3 one = vec3_new(1.0f, 1.0f, 1.0f);
return vec3_add(f0, vec3_scale(vec3_sub(one, f0), m));
}

float gbuffer_brdf_distribution_ggx(float n_dot_h, float roughness) {
    float a  = roughness * roughness;
    float a2 = a * a;
    float nh = saturate(n_dot_h);
    float d  = (nh * nh) * (a2 - 1.0f) + 1.0f;
    return a2 / (PI_F * d * d + 1e-7f);
}

static float geom_schlick_ggx(float n_dot_x, float k) {
    return n_dot_x / (n_dot_x * (1.0f - k) + k + 1e-7f);
}

float gbuffer_brdf_geometry_smith(float n_dot_v, float n_dot_l, float roughness) {
    // direct-light remap of roughness to k
    float r = roughness + 1.0f;
    float k = (r * r) / 8.0f;
    return geom_schlick_ggx(saturate(n_dot_v), k) *
           geom_schlick_ggx(saturate(n_dot_l), k);
}

vec3 gbuffer_brdf_eval(vec3 n, vec3 v, vec3 l, vec3 light_color,
                       vec3 albedo, gbuffer_material mat) {
    vec3 h = vec3_normalize(vec3_add(v, l));
float n_dot_l = saturate(vec3_dot(n, l));
if (n_dot_l <= 0.0f) return VEC3_ZERO;
float n_dot_v = saturate(vec3_dot(n, v));
float n_dot_h = saturate(vec3_dot(n, h));
float v_dot_h = saturate(vec3_dot(v, h));
vec3 dielectric = vec3_new(0.04f, 0.04f, 0.04f);
vec3 f0 = vec3_lerp(dielectric, albedo, mat.metallic);
float ndf = gbuffer_brdf_distribution_ggx(n_dot_h, mat.roughness);
float g   = gbuffer_brdf_geometry_smith(n_dot_v, n_dot_l, mat.roughness);
vec3  f   = gbuffer_brdf_fresnel(v_dot_h, f0);
float denom = 4.0f * n_dot_v * n_dot_l + 1e-4f;
vec3 spec = vec3_scale(f, ndf * g / denom);
vec3 one = vec3_new(1.0f, 1.0f, 1.0f);
vec3 kd  = vec3_scale(vec3_sub(one, f), 1.0f - mat.metallic);
vec3 diffuse = vec3_mul(kd, vec3_scale(albedo, 1.0f / PI_F));
vec3 brdf = vec3_add(diffuse, spec);
return vec3_mul(brdf, vec3_scale(light_color, n_dot_l));
