#include "ssr_resolve.h"
#include "ssr_fresnel.h"
#include "ssr_fade.h"
#include "ssr_config.h"

static float clamp01(float v) {
    return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

void ssrx_resolve_params_default(ssrx_resolve_params *p) {
    p->strength     = SSRX_DEFAULT_STRENGTH;
    p->edge_fade    = SSRX_DEFAULT_EDGE_FADE;
    p->dist_fade    = SSRX_DEFAULT_DIST_FADE;
    p->rough_cutoff = 0.6f;
    p->base_f0      = SSRX_DEFAULT_F0;
}

vec4 ssrx_resolve_fragment(const ssrx_march_result *hit,
                           const ssrx_ray *ray,
                           const ssrx_material *mat,
                           const ssrx_resolve_params *p,
                           ssrx_color_fn sample_color, void *user) {
    vec4 zero = vec4_new(0.0f, 0.0f, 0.0f, 0.0f);

    if (!ray->valid) return zero;
    if (hit->status != SSRX_MARCH_HIT) return zero;

    // roughness gate first — cheapest reject.
    float refl = ssrx_material_reflectivity(mat, p->rough_cutoff);
    if (refl <= 0.0f) return zero;

    // effective fresnel f0 for this surface (dielectric vs metal).
    float f0 = ssrx_fresnel_f0_metal(p->base_f0, mat->albedo, mat->metalness);
    // grazing-angle reflectance, damped by roughness.
    float fres = ssrx_fresnel_roughness(ray->n_dot_v, f0, mat->roughness);

    // confidence: edge + distance + backface, all in one.
    float conf = ssrx_fade_combine(hit->hit_uv, hit->t, ray->dir.z,
                                   p->edge_fade, p->dist_fade);
    if (conf <= 0.0f) return zero;

    // total weight. clamp so a hot fresnel * strength cant blow past 1.
    float w = fres * conf * refl * p->strength;
    w = clamp01(w);
    if (w <= 0.0f) return zero;

    // grab the lit scene color where the reflection landed.
    vec3 col = sample_color ? sample_color(user, hit->hit_uv)
                            : vec3_new(0.0f, 0.0f, 0.0f);

    // premultiplied: rgb*w in xyz, w in w. blends cleanly over the scene.
    return vec4_new(col.x * w, col.y * w, col.z * w, w);
}
