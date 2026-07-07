#ifndef RENDER_FXAA_QUALITY_H
#define RENDER_FXAA_QUALITY_H

#include "fxaa_config.h"

// quality presets for the edge search. the inner loop walks left/right (or
// up/down) along an edge sampling luma every few texels until it falls off
// the end of the span. each preset is a list of step *distances* — the gaps
// between successive samples. nvidia's reference fxaa shipped these as the
// FXAA_QUALITY__Pn macros; we keep them as a real table so the search code
// is a plain loop instead of a macro pyramid.
//
// the pattern is: tight steps near the origin (sub-texel accuracy where it
// matters) then increasingly coarse steps far out (cheap reach for long
// shallow edges). the last entry is usually a big jump.

typedef struct {
    int          step_count;          // number of search iterations per side
    const float *steps;               // step distance for each iteration
    float        last_edge_guess;     // fallback distance if we never find an end
} fxaa_quality;

// fetch the preset for a FXAA_QUALITY_* index. out-of-range clamps to the
// nearest valid preset (never returns a null steps pointer).
const fxaa_quality *fxaa_quality_get(int preset);

// total reach in texels for a preset (sum of its step distances). handy for
// the debug overlay and for picking texture clamp margins.
float fxaa_quality_reach(const fxaa_quality *q);

// human label for the preset index, for logs / ui. always non-null.
const char *fxaa_quality_name(int preset);

#endif
