#include "fxaa_settings.h"

#include <math.h>
#include <stddef.h>

void fxaa_settings_apply(fxaa_params *p, fxaa_setting s) {
    // start from sane defaults, then override the knobs that matter per level.
    fxaa_params_default(p);

    switch (s) {
    case FXAA_SET_OFF:
        p->enabled = 0;
        break;
    case FXAA_SET_FAST:
        // only the harshest edges, minimal search, barely any subpix. cheapest.
        p->edge_threshold = 0.25f;
        p->subpix         = 0.25f;
        p->quality        = FXAA_QUALITY_LOW;
        break;
    case FXAA_SET_DEFAULT:
        p->edge_threshold = FXAA_DEFAULT_EDGE_THRESHOLD;
        p->subpix         = FXAA_DEFAULT_SUBPIX;
        p->quality        = FXAA_QUALITY_HIGH;
        break;
    case FXAA_SET_QUALITY:
        p->edge_threshold = 0.125f;
        p->subpix         = 0.85f;
        p->quality        = FXAA_QUALITY_HIGH;
        break;
    case FXAA_SET_EXTREME:
        // catch nearly everything, longest search, full subpix. screenshot mode.
        p->edge_threshold = 0.0625f;
        p->subpix         = 1.0f;
        p->quality        = FXAA_QUALITY_EXTREME;
        break;
    default:
        // unknown -> default. dont leave the params half-initialised.
        p->edge_threshold = FXAA_DEFAULT_EDGE_THRESHOLD;
        p->subpix         = FXAA_DEFAULT_SUBPIX;
        p->quality        = FXAA_QUALITY_HIGH;
        break;
    }
    fxaa_params_sanitize(p);
}

fxaa_setting fxaa_settings_classify(const fxaa_params *p) {
    if (!p->enabled) return FXAA_SET_OFF;

    // score each preset's reference params against the current ones and pick
    // the nearest. threshold weighs more than subpix because it's the bigger
    // visual lever.
    fxaa_setting best = FXAA_SET_DEFAULT;
    float best_d = 1e9f;
    for (fxaa_setting s = FXAA_SET_FAST; s < FXAA_SET_COUNT; s++) {
        fxaa_params ref;
        fxaa_settings_apply(&ref, s);
        if (!ref.enabled) continue;
        float dt = (p->edge_threshold - ref.edge_threshold);
        float ds = (p->subpix - ref.subpix);
        float d = fabsf(dt) * 4.0f + fabsf(ds);
        if (d < best_d) { best_d = d; best = s; }
    }
    return best;
}

const char *fxaa_settings_name(fxaa_setting s) {
    switch (s) {
    case FXAA_SET_OFF:     return "off";
    case FXAA_SET_FAST:    return "fast";
    case FXAA_SET_DEFAULT: return "default";
    case FXAA_SET_QUALITY: return "quality";
    case FXAA_SET_EXTREME: return "extreme";
    default:               return "?";
    }
}

fxaa_setting fxaa_settings_cycle(fxaa_params *p, fxaa_setting cur, int dir) {
    int n = (int)cur + (dir > 0 ? 1 : -1);
    if (n < 0) n = 0;
    if (n >= FXAA_SET_COUNT) n = FXAA_SET_COUNT - 1;
    fxaa_settings_apply(p, (fxaa_setting)n);
    return (fxaa_setting)n;
}
