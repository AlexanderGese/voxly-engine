#include "ssr_settings.h"
#include "ssr_config.h"

#include <stdio.h>

static int   clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }
static float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

void ssrx_settings_default(ssrx_settings *s) {
    s->enabled      = 1;
    s->scale        = SSRX_DEFAULT_SCALE;

    s->max_steps    = SSRX_DEFAULT_STEPS;
    s->refine_steps = SSRX_DEFAULT_REFINE;
    s->stride       = SSRX_DEFAULT_STRIDE;
    s->growth       = SSRX_STRIDE_GROWTH;
    s->thickness    = SSRX_DEFAULT_THICKNESS;

    s->strength     = SSRX_DEFAULT_STRENGTH;
    s->edge_fade    = SSRX_DEFAULT_EDGE_FADE;
    s->dist_fade    = SSRX_DEFAULT_DIST_FADE;
    s->rough_cutoff = 0.6f;
    s->base_f0      = SSRX_DEFAULT_F0;
}

void ssrx_settings_clamp(ssrx_settings *s) {
    s->scale        = clampi(s->scale, 1, 4);
    s->max_steps    = clampi(s->max_steps, 1, SSRX_MAX_STEPS);
    s->refine_steps = clampi(s->refine_steps, 0, SSRX_MAX_REFINE);
    s->stride       = clampf(s->stride, 0.01f, 4.0f);
    s->growth       = clampf(s->growth, 1.0f, 1.5f);
    s->thickness    = clampf(s->thickness, 0.01f, 8.0f);

    s->strength     = clampf(s->strength, 0.0f, 1.0f);
    s->edge_fade    = clampf(s->edge_fade, 0.0f, 0.5f);
    s->dist_fade    = clampf(s->dist_fade, 0.0f, 1.0f);
    s->rough_cutoff = clampf(s->rough_cutoff, 0.01f, 1.0f);
    s->base_f0      = clampf(s->base_f0, 0.0f, 1.0f);
}

void ssrx_settings_to_march(const ssrx_settings *s, ssrx_march_params *out) {
    out->max_steps    = s->max_steps;
    out->refine_steps = s->refine_steps;
    out->stride       = s->stride;
    out->growth       = s->growth;
    out->thickness    = s->thickness;
}

void ssrx_settings_to_resolve(const ssrx_settings *s, ssrx_resolve_params *out) {
    out->strength     = s->strength;
    out->edge_fade    = s->edge_fade;
    out->dist_fade    = s->dist_fade;
    out->rough_cutoff = s->rough_cutoff;
    out->base_f0      = s->base_f0;
}

int ssrx_settings_describe(const ssrx_settings *s, char *buf, int cap) {
    if (cap <= 0) return 0;
    int n = snprintf(buf, (size_t)cap,
                     "ssr %s 1/%d steps=%d refine=%d thick=%.2f str=%.2f edge=%.2f",
                     s->enabled ? "on" : "off",
                     s->scale, s->max_steps, s->refine_steps,
                     s->thickness, s->strength, s->edge_fade);
    if (n < 0) return 0;
    if (n >= cap) n = cap - 1;   // snprintf truncated; report what fit
    return n;
}
