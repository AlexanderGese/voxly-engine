#ifndef RENDER_SKYBOX_SCATTER_H
#define RENDER_SKYBOX_SCATTER_H

// a cheap analytic scattering term for the sky glow. NOT real rayleigh/mie —
// it's a two-lobe phase function (a wide rayleigh-ish lobe + a tight forward
// mie-ish lobe) over a fake optical-depth curve by altitude. it gives the sky
// near the sun a believable bright-then-warm falloff and the anti-sun side a
// faint lift, without the cost of a real LUT.
//
// the gradient module can fold this in instead of its hand-rolled glow when
// you want the look to track sun height more physically. opt-in.

#include "skyb_common.h"

typedef struct {
    float rayleigh;    // strength of the wide blue-ish lobe
    float mie;         // strength of the tight forward lobe (sun halo)
    float mie_g;       // mie asymmetry -1..1 (0.76 is a good forward bias)
    float turbidity;   // 1..10 haze amount; raises mie + reddens low sun
} skyb_scatter;

// sane clear-day defaults.
void skyb_scatter_default(skyb_scatter *s);

// the rayleigh phase: 0.75*(1+cos^2). normalized-ish, returns ~0.75..1.5.
float skyb_phase_rayleigh(float cos_theta);

// henyey-greenstein phase for the mie lobe. g is asymmetry.
float skyb_phase_mie(float cos_theta, float g);

// optical depth proxy along a view dir. low to the horizon (long path),
// near-zero at zenith. used to redden + dim the low sky.
float skyb_optical_depth(vec3 view_dir);

// evaluate the added scatter color for a view dir given the sun dir + the
// sun's tint and light level. returns an additive rgb to mix onto the base
// gradient. self-clamped to [0,1] per channel.
skyb_rgb skyb_scatter_eval(const skyb_scatter *s, vec3 view_dir,
                           vec3 sun_dir, skyb_rgb sun_tint, float sun_light);

#endif
