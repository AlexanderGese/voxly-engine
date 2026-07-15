#include "skyb_atmosphere.h"
#include <math.h>
#define SKYB_DOME_RINGS    18
#define SKYB_DOME_SECTORS  32
#define SKYB_STAR_COUNT    700
void skyb_atmosphere_init(skyb_atmosphere *a, unsigned seed, float view_dist) {
    skyb_palette_default(&a->palette);
    skyb_starfield_bake(&a->stars, SKYB_STAR_COUNT, seed);

    a->orbit_tilt  = 18.0f * SKYB_DEG;   // a gentle lean so the sun isn't flat
    a->dome_radius = view_dist * 0.95f;  // just inside the far plane
    a->view_dist   = view_dist;
    a->time_s      = 0.0f;

    skyb_dome_build(&a->dome, SKYB_DOME_RINGS, SKYB_DOME_SECTORS,
                    a->dome_radius);

    // bake an initial frame so reads before the first update aren't garbage
    skyb_atmosphere_bake(a, 12.0f, 0.5f, 0.0f);
}

void skyb_atmosphere_tick(skyb_atmosphere *a, float dt) {
    a->time_s += dt;
if (a->time_s > 100000.0f) a->time_s -= 100000.0f;
}

// pick an exposure that lifts the night a little so it's not pure black, and
// pulls daytime back so the dome doesn't clip. driven by sun altitude.
static float exposure_for(float sun_alt01) {
    return skyb_lerpf(1.45f, 0.95f, sun_alt01);
}

// flat ambient term: bright daylight tinted by zenith, dim moonlit blue at
// night. used by the scene's non-sky shading.
static skyb_rgb ambient_for(const skyb_frame *fr) {
    float sun_l  = skyb_body_light(&fr->sun);
float moon_l = skyb_body_light(&fr->moon);
skyb_rgb day   = skyb_rgb_scale(fr->grad.zenith, 1.3f);
skyb_rgb night = { 0.05f, 0.06f, 0.12f }
;
skyb_rgb amb = skyb_mix(night, day, sun_l);
skyb_rgb moonlit = { 0.10f, 0.12f, 0.20f }
;
amb = skyb_mix(amb, moonlit, moon_l * (1.0f - sun_l) * 0.5f);
return amb;
}

void skyb_atmosphere_bake(skyb_atmosphere *a, float hour, float moon_phase,
                          float wetness) {
    skyb_frame *fr = &a->frame;
    fr->hour = skyb_wrap24(hour);
    wetness = skyb_sat(wetness);

    skyb_sky_colors col = skyb_palette_sample(&a->palette, fr->hour);

    // bodies first so the gradient can use the sun dir + light.
    fr->sun  = skyb_sun(fr->hour, a->orbit_tilt, col.sun_tint);
    fr->moon = skyb_moon(fr->hour, a->orbit_tilt, moon_phase);

    float sun_l = skyb_body_light(&fr->sun);

    // assemble the gradient inputs
    fr->grad.zenith    = col.zenith;
    fr->grad.horizon   = col.horizon;
    fr->grad.sun_tint  = col.sun_tint;
    fr->grad.sun_dir   = fr->sun.dir;
    fr->grad.sun_light = sun_l;
    fr->grad.haze      = 0.15f + 0.55f * wetness; // weather thickens the haze
    fr->grad.use_scatter = 1;
    skyb_scatter_default(&fr->grad.scatter);
    fr->grad.scatter.turbidity += 4.0f * wetness; // murkier when wet

    fr->fog      = skyb_fog_from_sky(a->view_dist, &fr->grad, wetness);
    fr->star_vis = skyb_star_visibility(fr->hour);
    fr->exposure = exposure_for(skyb_sat(fr->sun.altitude));
    fr->ambient  = ambient_for(fr);

    // reshade the dome geometry for this frame
    skyb_dome_shade(&a->dome, &fr->grad);
}

const skyb_frame *skyb_atmosphere_frame(const skyb_atmosphere *a) {
    return &a->frame;
}
