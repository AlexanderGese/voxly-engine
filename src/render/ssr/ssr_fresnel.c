#include "ssr_fresnel.h"
#include "ssr_config.h"

#include <math.h>

static float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

float ssrx_fresnel_schlick(float cos_theta, float f0) {
    float c = clampf(cos_theta, 0.0f, 1.0f);
    float m = 1.0f - c;
    // m^5 without calling powf five thousand times a frame.
    float m2 = m * m;
    float m5 = m2 * m2 * m;
    return f0 + (1.0f - f0) * m5;
}

float ssrx_fresnel_clamped(float cos_theta, float f0) {
    float f = ssrx_fresnel_schlick(cos_theta, f0);
    return clampf(f, SSRX_FRESNEL_MIN, SSRX_FRESNEL_MAX);
}

float ssrx_fresnel_f0_metal(float base_f0, vec3 albedo, float metalness) {
    float m = clampf(metalness, 0.0f, 1.0f);
    // perceived albedo weight. rec.709-ish luma; good enough, we only need a
    // monochrome reflectance for the blend.
    float luma = 0.2126f * albedo.x + 0.7152f * albedo.y + 0.0722f * albedo.z;
    luma = clampf(luma, 0.0f, 1.0f);
    // dielectric uses base_f0; metal uses its own albedo as f0.
    return base_f0 * (1.0f - m) + luma * m;
}

float ssrx_fresnel_roughness(float cos_theta, float f0, float roughness) {
    float c   = clampf(cos_theta, 0.0f, 1.0f);
    float r   = clampf(roughness, 0.0f, 1.0f);
    // f90 drops from 1 toward f0 as roughness rises, so rough surfaces lose
    // their grazing rim. max(1-r, f0) keeps smooth surfaces at a full rim.
    float f90 = (1.0f - r) > f0 ? (1.0f - r) : f0;
    float m   = 1.0f - c;
    float m2  = m * m;
    float m5  = m2 * m2 * m;
    return f0 + (f90 - f0) * m5;
}
