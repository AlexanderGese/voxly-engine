#include "ssr_material.h"
#include "ssr_config.h"

static float clamp01(float v) {
    return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

ssrx_material ssrx_material_default(void) {
    ssrx_material m;
    m.roughness = 0.25f;            // a touch glossy, not a perfect mirror
    m.metalness = 0.0f;
    m.albedo    = vec3_new(0.5f, 0.5f, 0.5f);
    return m;
}

ssrx_material ssrx_material_unpack(float r, float g, vec3 albedo) {
    ssrx_material m;
    m.roughness = clamp01(r);
    m.metalness = clamp01(g);
    m.albedo.x  = clamp01(albedo.x);
    m.albedo.y  = clamp01(albedo.y);
    m.albedo.z  = clamp01(albedo.z);
    return m;
}

float ssrx_material_reflectivity(const ssrx_material *m, float cutoff) {
    if (cutoff <= 0.0f) cutoff = 1.0f;
    // linear taper from full reflectivity at roughness 0 to nothing at cutoff.
    if (m->roughness >= cutoff) return 0.0f;
    float w = 1.0f - (m->roughness / cutoff);
    // metals stay reflective a bit longer into the rough range — bias up.
    w += m->metalness * 0.25f * (1.0f - w);
    return clamp01(w);
}
