#ifndef RENDER_BLOOM2_PARAMS_H
#define RENDER_BLOOM2_PARAMS_H

#include "bloom2_config.h"

// runtime knobs. these are separate from the static config defines so the
// debug ui / console can poke at them without a rebuild.

typedef struct {
    float threshold;   // luma below this contributes nothing
    float knee;        // soft-knee width around the threshold (0 = hard)
    float intensity;   // additive strength at composite
    float radius;      // upsample tent radius in mip texels
    float clamp_max;   // firefly clamp on the bright pass
    int   mip_count;   // how many mips the chain will actually use (<= MAX)
    int   enabled;     // master on/off
} bloom2_params;

// fill p with the compile-time defaults.
void bloom2_params_default(bloom2_params *p);

// clamp everything into legal ranges. ui sliders are sloppy, do this before
// the params hit the gpu so we dont feed negative thresholds etc.
void bloom2_params_sanitize(bloom2_params *p);

// the bright pass uses a soft knee curve. precompute the (x, y, z, w) it
// wants from threshold+knee so the shader can do a couple of cheap maxes
// instead of branching. unreal's curve, basically.
// out[0] = threshold
// out[1] = threshold - knee
// out[2] = 2 * knee
// out[3] = 0.25 / (knee + 1e-5)
void bloom2_params_knee_curve(const bloom2_params *p, float out[4]);

#endif
