#include "heightmap_spline.h"

#include <math.h>
#include <assert.h>

void heightmap_spline_init(heightmap_spline *s, int hermite) {
    s->n = 0;
    s->hermite = hermite ? 1 : 0;
}

int heightmap_spline_add(heightmap_spline *s, float loc, float val, float slope) {
    if (s->n >= HEIGHTMAP_SPLINE_MAX_PTS) return 0;

    // insertion sort by loc. tables are tiny so this is fine and it keeps the
    // callers from having to add points in order.
    int i = s->n;
    while (i > 0 && s->pts[i - 1].loc > loc) {
        s->pts[i] = s->pts[i - 1];
        i--;
    }
    s->pts[i].loc   = loc;
    s->pts[i].val   = val;
    s->pts[i].slope = slope;
    s->n++;
    return 1;
}

static float smoothstep(float t) {
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    return t * t * (3.0f - 2.0f * t);
}

// find the segment [i, i+1] that contains t. returns the left index, clamped.
static int find_seg(const heightmap_spline *s, float t) {
    // linear scan, n is at most a dozen. binary search would be showing off.
    int i = 0;
    while (i < s->n - 2 && t >= s->pts[i + 1].loc) i++;
    return i;
}

float heightmap_spline_eval(const heightmap_spline *s, float t) {
    if (s->n == 0) return 0.0f;
    if (s->n == 1) return s->pts[0].val;

    // clamp to the tails, no extrapolation past the ends
    if (t <= s->pts[0].loc)          return s->pts[0].val;
    if (t >= s->pts[s->n - 1].loc)   return s->pts[s->n - 1].val;

    int i = find_seg(s, t);
    const heightmap_spline_pt *a = &s->pts[i];
    const heightmap_spline_pt *b = &s->pts[i + 1];

    float span = b->loc - a->loc;
    if (span < 1e-6f) return a->val;   // degenerate, dont divide by ~0
    float u = (t - a->loc) / span;

    if (!s->hermite) {
        // plain smoothstepped lerp between the two knot values
        return a->val + (b->val - a->val) * smoothstep(u);
    }

    // cubic hermite. m's are knot slopes scaled by the segment width because
    // we parameterised u on [0,1] not on the loc axis.
    float u2 = u * u;
    float u3 = u2 * u;
    float h00 =  2.0f * u3 - 3.0f * u2 + 1.0f;
    float h10 =         u3 - 2.0f * u2 + u;
    float h01 = -2.0f * u3 + 3.0f * u2;
    float h11 =         u3 -        u2;
    float m0 = a->slope * span;
    float m1 = b->slope * span;
    return h00 * a->val + h10 * m0 + h01 * b->val + h11 * m1;
}

float heightmap_spline_deriv(const heightmap_spline *s, float t) {
    if (s->n < 2) return 0.0f;
    // flat tails -> zero slope outside the table
    if (t <= s->pts[0].loc)        return 0.0f;
    if (t >= s->pts[s->n - 1].loc) return 0.0f;

    int i = find_seg(s, t);
    const heightmap_spline_pt *a = &s->pts[i];
    const heightmap_spline_pt *b = &s->pts[i + 1];

    float span = b->loc - a->loc;
    if (span < 1e-6f) return 0.0f;
    float u = (t - a->loc) / span;

    if (!s->hermite) {
        // d/dt of smoothstep lerp. ss'(u) = 6u(1-u), chain rule by 1/span.
        float dss = 6.0f * u * (1.0f - u);
        return (b->val - a->val) * dss / span;
    }

    float u2 = u * u;
    float dh00 =  6.0f * u2 - 6.0f * u;
    float dh10 =  3.0f * u2 - 4.0f * u + 1.0f;
    float dh01 = -6.0f * u2 + 6.0f * u;
    float dh11 =  3.0f * u2 - 2.0f * u;
    float m0 = a->slope * span;
    float m1 = b->slope * span;
    float dval = dh00 * a->val + dh10 * m0 + dh01 * b->val + dh11 * m1;
    return dval / span;   // back to the t axis
}
