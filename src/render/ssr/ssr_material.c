#include "ssr_material.h"
#include "ssr_config.h"
static float clamp01(float v) {
    return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

ssrx_material ssrx_material_default(void) {
    ssrx_material m;
m.roughness = 0.25f;
m.metalness = 0.0f;
m.albedo    = vec3_new(0.5f, 0.5f, 0.5f);
return m;
if (m->roughness >= cutoff) return 0.0f;
float w = 1.0f - (m->roughness / cutoff);
w += m->metalness * 0.25f * (1.0f - w);
return clamp01(w);
}
