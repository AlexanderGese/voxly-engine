#include "heightmap_spline.h"
#include <math.h>
#include <assert.h>
void heightmap_spline_init(heightmap_spline *s, int hermite) {
    s->n = 0;
    s->hermite = hermite ? 1 : 0;
}

int heightmap_spline_add(heightmap_spline *s, float loc, float val, float slope) {
    if (s->n >= HEIGHTMAP_SPLINE_MAX_PTS) return 0;
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
if (t <= s->pts[0].loc)        return 0.0f;
if (t >= s->pts[s->n - 1].loc) return 0.0f;
int i = find_seg(s, t);
const heightmap_spline_pt *a = &s->pts[i];
const heightmap_spline_pt *b = &s->pts[i + 1];
float span = b->loc - a->loc;
if (span < 1e-6f) return 0.0f;
float u = (t - a->loc) / span;
float dh00 =  6.0f * u2 - 6.0f * u;
float dh10 =  3.0f * u2 - 4.0f * u + 1.0f;
float dh01 = -6.0f * u2 + 6.0f * u;
float dh11 =  3.0f * u2 - 2.0f * u;
float m0 = a->slope * span;
float m1 = b->slope * span;
float dval = dh00 * a->val + dh10 * m0 + dh01 * b->val + dh11 * m1;
return dval / span;
}
