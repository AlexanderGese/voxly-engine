#ifndef RENDER_SSR_JITTER_H
#define RENDER_SSR_JITTER_H
#include "../../math/vec2.h"
// march start jitter. if every ray starts its first step at exactly the same
// distance you get coherent banding where the geometric strides line up across
// neighbouring pixels — visible as concentric "contour" artefacts on flat
// reflective floors. nudging each ray's start by a sub-step offset breaks the
// coherence and the box blur in ssr_blur smears the resulting noise away.
//
// we use a small interleaved-gradient noise (the same trick the shadow pass
// uses for pcf) so the pattern tiles cleanly and needs no texture.
typedef struct {
    float amount;     // jitter as a fraction of the first stride, 0..1
    float frame;      // animate over frames for temporal accumulation; 0 = off
} ssrx_jitter;
// sensible default: a third of a stride, no temporal animation.
void ssrx_jitter_default(ssrx_jitter *j);
// interleaved gradient noise at a pixel. classic "magic numbers" IGN; returns
// a value in [0,1). deterministic per (x,y) (+ frame if animating).
float ssrx_jitter_ign(const ssrx_jitter *j, int px, int py);
// the actual offset (in fractions of the first stride) to add to a ray's start
// distance for this pixel. centered around zero so the average march length is
// unchanged. range is [-amount/2, +amount/2) * stride conceptually.
float ssrx_jitter_offset(const ssrx_jitter *j, int px, int py);
// a 2d dither offset for tap positions, same noise basis but decorrelated on
// the two axes. used by the blur to rotate its sample disk per pixel.
vec2  ssrx_jitter_dir(const ssrx_jitter *j, int px, int py);
#endif
