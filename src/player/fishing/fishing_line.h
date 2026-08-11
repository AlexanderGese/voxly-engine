#ifndef PLAYER_FISHING_LINE_H
#define PLAYER_FISHING_LINE_H
#include "fishing_types.h"
#include "../../math/vec3.h"
// the visible line between the rod tip and the bobber. we don't draw it here
// (no gl in this module on purpose) — we just sample a catenary-ish curve so
// whoever owns the renderer can push a strip. the sag scales with how slack the
// line is, so a tight reel pulls it straight and an idle cast droops.
#define FISHING_LINE_SEGMENTS  16   // points = segments+1
typedef struct {
    vec3  points[FISHING_LINE_SEGMENTS + 1];
    int   count;                 // always SEGMENTS+1, kept for the renderer
    float length;                // straight-line rod->bobber distance
} fishing_line;
// build the curve from `rod` (tip) to `bob`. `slack` 0..1 controls droop: 0 is
// taut, 1 is a lazy cast hanging in the air. fills out->points.
void  fishing_line_build(fishing_line *out, vec3 rod, vec3 bob, float slack);
// total arc length of the sampled curve. always >= the straight distance.
float fishing_line_arc_length(const fishing_line *l);
float fishing_line_slack_from_tension(float tension_frac);
#endif
