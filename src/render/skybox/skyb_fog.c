#include "skyb_fog.h"

#include <math.h>

skyb_fog skyb_fog_from_sky(float view_dist, const skyb_gradient *grad,
                           float wetness) {
    skyb_fog f;
    wetness = skyb_sat(wetness);

    // clear weather: fog only really bites in the last third of view distance.
    // rain/snow pulls it way in and thickens it.
    float clear_start = view_dist * 0.62f;
    float wet_start   = view_dist * 0.20f;
    f.start = skyb_lerpf(clear_start, wet_start, wetness);
    f.end   = view_dist * skyb_lerpf(0.98f, 0.80f, wetness);

    f.exp_mode = wetness > 0.35f; // heavy weather -> exp2 falloff, softer edge
    // density chosen so exp2 reaches ~0.95 near `end`
    float span = (f.end > 1.0f) ? f.end : 1.0f;
    f.density = (1.6f + 2.5f * wetness) / span;

    // base color: the horizon color is the sane fallback when we can't sample
    // a view dir (it's where most fog is seen anyway).
    f.base = grad ? grad->horizon : (skyb_rgb){0.7f, 0.8f, 0.9f};
    return f;
}

float skyb_fog_factor(const skyb_fog *f, float dist) {
    if (dist <= f->start) return 0.0f;
    if (f->exp_mode) {
        float d = (dist - f->start) * f->density;
        float a = 1.0f - expf(-d * d);     // exp2
        return skyb_sat(a);
    }
    // linear ramp start->end
    return skyb_smooth(f->start, f->end, dist);
}

skyb_rgb skyb_fog_color(const skyb_fog *f, const skyb_gradient *grad,
                        vec3 view_dir) {
    if (!grad) return f->base;
    // sample the sky a touch above the actual view dir so fog tends toward the
    // brighter sky band rather than the dark ground line — looks less muddy.
    vec3 d = vec3_normalize(view_dir);
    d.y = skyb_clampf(d.y + 0.05f, -1.0f, 1.0f);
    return skyb_gradient_eval(grad, d);
}

skyb_rgb skyb_fog_apply(const skyb_fog *f, const skyb_gradient *grad,
                        skyb_rgb surface, vec3 view_dir, float dist) {
    float a = skyb_fog_factor(f, dist);
    if (a <= 0.0f) return surface;
    skyb_rgb fog = skyb_fog_color(f, grad, view_dir);
    return skyb_mix(surface, fog, a);
}
