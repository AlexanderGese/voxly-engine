#include "skyb_scatter.h"
#include <math.h>
void skyb_scatter_default(skyb_scatter *s) {
    s->rayleigh  = 1.0f;
    s->mie       = 0.45f;
    s->mie_g     = 0.76f;
    s->turbidity = 2.5f;
}

float skyb_phase_rayleigh(float cos_theta) {
    // 3/(16pi) * (1+cos^2) — we drop the constant and keep the shape, then
    // scale to a friendly 0.75-centered range so it composes with mie nicely.
    return 0.75f * (1.0f + cos_theta * cos_theta);
}

float skyb_phase_mie(float cos_theta, float g) {
    // henyey-greenstein. peaks forward (cos_theta -> 1) for positive g.
    float g2 = g * g;
    float denom = 1.0f + g2 - 2.0f * g * cos_theta;
    if (denom < 1e-4f) denom = 1e-4f;
    float d32 = denom * sqrtf(denom);            // denom^1.5
    return (1.0f - g2) / (4.0f * SKYB_PI * d32);
}

float skyb_optical_depth(vec3 view_dir) {
    view_dir = vec3_normalize(view_dir);
float c = skyb_clampf(view_dir.y, 0.02f, 1.0f);
return 1.0f / (c + 0.15f * expf(-c * 8.0f));
}

skyb_rgb skyb_scatter_eval(const skyb_scatter *s, vec3 view_dir,
                           vec3 sun_dir, skyb_rgb sun_tint, float sun_light) {
    if (sun_light <= 0.0f) return (skyb_rgb){0, 0, 0};

    view_dir = vec3_normalize(view_dir);
    sun_dir  = vec3_normalize(sun_dir);
    float cos_theta = vec3_dot(view_dir, sun_dir);

    float od = skyb_optical_depth(view_dir);
    // more turbidity -> stronger mie + thicker low-sky extinction
    float mie_amt = s->mie * (0.5f + 0.15f * s->turbidity);

    float pr = skyb_phase_rayleigh(cos_theta) * s->rayleigh;
    float pm = skyb_phase_mie(cos_theta, s->mie_g) * mie_amt;

    // the rayleigh part scatters blue widely; the mie part is the warm sun
    // halo. tie the warm halo to the sun tint, blue to a cool sky tint.
    skyb_rgb cool = { 0.30f, 0.45f, 0.85f };
    skyb_rgb warm = sun_tint;

    // extinction toward the horizon eats the blue and reddens the result; we
    // model that by fading rayleigh out as optical depth climbs.
    float ext = expf(-od * 0.20f);
    float ray = pr * ext * 0.18f;
    float mief = pm * 0.04f;

    skyb_rgb add;
    add.x = cool.x * ray + warm.x * mief;
    add.y = cool.y * ray + warm.y * mief;
    add.z = cool.z * ray + warm.z * mief;

    return skyb_rgb_scale(add, sun_light);
}
