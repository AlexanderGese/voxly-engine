#ifndef RENDER_FXAA_PARAMS_H
#define RENDER_FXAA_PARAMS_H

#include "fxaa_config.h"

// runtime knobs, separate from the static config defines so the debug ui /
// console can poke them live without a rebuild. the defaults come from
// fxaa_config.h but everything here is clamped before it touches the gpu.

typedef struct {
    float edge_threshold;      // relative local-contrast trigger
    float edge_threshold_min;  // absolute contrast floor (dark-region guard)
    float subpix;              // sub-pixel aliasing removal strength [0..1]
    int   quality;             // FXAA_QUALITY_* preset index
    int   enabled;             // master on/off
    int   show_edges;          // debug: visualise the edge mask instead
} fxaa_params;

// fill p with the compile-time defaults.
void fxaa_params_default(fxaa_params *p);

// clamp everything into legal ranges. sliders are sloppy; do this before the
// params hit the shader so we dont feed a negative threshold or a quality
// index that walks off the end of the table.
void fxaa_params_sanitize(fxaa_params *p);

// fxaa's main shader wants the subpix strength split into a couple of derived
// scalars so the inner loop can avoid a divide. precompute them here.
// out[0] = subpix                 (raw strength)
// out[1] = subpix * subpix * 0.5  (quadratic falloff used on thin features)
// out[2] = 1.0 / edge_threshold   (reciprocal for the trigger compare)
// out[3] = edge_threshold_min
void fxaa_params_derive(const fxaa_params *p, float out[4]);

// true if the params would actually do any work this frame.
int  fxaa_params_active(const fxaa_params *p);

#endif
