#ifndef RENDER_SHADOW_SPLIT_H
#define RENDER_SHADOW_SPLIT_H
#include "shadow_types.h"
// cascade split selection. the "practical split scheme" (zhang et al) — a
// blend of uniform and logarithmic distribution controlled by lambda.
// fill out->splits[0..count] given the camera near/far we actually want to
// shadow. clamps far to SHADOW_MAX_DISTANCE internally.
void shadow_split_compute(shadow_csm *out, float cam_near, float cam_far);
// pick which cascade a view-space depth lands in. returns count-1 if it's
// past the last split (i.e. shadowed by the coarsest cascade).
int  shadow_split_select(const shadow_csm *csm, float view_depth);
// the raw practical-split helper, exposed for tests. i = 0..count.
float shadow_split_practical(int i, int count, float near, float far, float lambda);
// fraction of the [near,far] range this cascade covers. handy for debug bars.
float shadow_split_fraction(const shadow_csm *csm, int cascade);
// a rough "texel error" score for the current splits: how uneven the world-
// space texel size is across cascades. lower is better-balanced. used by the
// auto-tuner below and by the debug overlay.
float shadow_split_error(const shadow_csm *csm, int map_size);
// search lambda in [0,1] for the value that minimizes shadow_split_error and
// rewrite csm->splits with it. cheap golden-section search, ~20 evals. call
// it once when render distance changes, not per frame.
float shadow_split_autotune(shadow_csm *csm, int map_size,
                            float cam_near, float cam_far);
#endif
