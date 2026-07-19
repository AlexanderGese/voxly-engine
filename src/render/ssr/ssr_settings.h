#ifndef RENDER_SSR_SETTINGS_H
#define RENDER_SSR_SETTINGS_H

#include "ssr_march.h"
#include "ssr_resolve.h"

// the runtime-mutable bag of ssr knobs. the pass owns one of these and the
// debug ui poke at it live. split out from the pass so tests can spin one up
// without touching gl. seeded from the config defaults; clamps keep the debug
// sliders from putting it in a stupid state.

typedef struct {
    int   enabled;        // master on/off (separate from "shaders missing")
    int   scale;          // gbuffer downsample factor for the reflect buffer

    // march
    int   max_steps;
    int   refine_steps;
    float stride;
    float growth;
    float thickness;

    // resolve
    float strength;
    float edge_fade;
    float dist_fade;
    float rough_cutoff;
    float base_f0;
} ssrx_settings;

// load every field from the SSRX_DEFAULT_* config values.
void ssrx_settings_default(ssrx_settings *s);

// clamp all fields back into sane ranges. call after the ui mutates them.
void ssrx_settings_clamp(ssrx_settings *s);

// project the settings onto the two param structs the cpu path consumes.
void ssrx_settings_to_march(const ssrx_settings *s, ssrx_march_params *out);
void ssrx_settings_to_resolve(const ssrx_settings *s, ssrx_resolve_params *out);

// a one-line human summary into `buf` (for the debug overlay / logs). returns
// the number of chars written (excluding the nul), like snprintf.
int  ssrx_settings_describe(const ssrx_settings *s, char *buf, int cap);

#endif
