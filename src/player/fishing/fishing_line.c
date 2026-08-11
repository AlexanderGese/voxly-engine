#include "fishing_line.h"
#include <math.h>

// max droop in world units at full slack for a unit-length line. the actual sag
// scales with line length so a long cast hangs lower than a short one.
#define LINE_SAG_SCALE  0.35f

void fishing_line_build(fishing_line *out, vec3 rod, vec3 bob, float slack) {
    if (slack < 0.0f) slack = 0.0f;
    if (slack > 1.0f) slack = 1.0f;

    float len = vec3_distance(rod, bob);
    out->length = len;
    out->count  = FISHING_LINE_SEGMENTS + 1;

    // sag amount: zero at the endpoints, peaking mid-span. a real catenary is a
    // cosh, but a parabola reads identically at this scale and is cheaper.
    float sag = LINE_SAG_SCALE * len * slack;

    for (int i = 0; i <= FISHING_LINE_SEGMENTS; i++) {
        float t = (float)i / (float)FISHING_LINE_SEGMENTS;
        // straight interpolation rod->bob...
        vec3 p = vec3_lerp(rod, bob, t);
        // ...then pull the middle down. 4t(1-t) is a clean 0->1->0 hump.
        float droop = 4.0f * t * (1.0f - t) * sag;
        p.y -= droop;
        out->points[i] = p;
    }
}

float fishing_line_arc_length(const fishing_line *l) {
    float total = 0.0f;
    for (int i = 1; i < l->count; i++)
        total += vec3_distance(l->points[i - 1], l->points[i]);
    return total;
}

float fishing_line_slack_from_tension(float tension_frac) {
    if (tension_frac < 0.0f) tension_frac = 0.0f;
    if (tension_frac > 1.0f) tension_frac = 1.0f;
    // taut line at high tension, droopy when slack. invert and ease a little so
    // the line snaps straight quickly as you start winding.
    float s = 1.0f - tension_frac;
    return s * s;
}
