#ifndef RENDER_SHADOW_BOUNDS_H
#define RENDER_SHADOW_BOUNDS_H

#include "shadow_types.h"

// turn a slice's frustum corners into a tight, stable light-space ortho box.
// "stable" is the operative word — naive fitting makes the shadows crawl as
// the camera moves. we fit to a bounding sphere and snap to texel grid.

// compute the ortho extents (in light space) and depth range for one cascade.
// fills cascade->bounds and cascade->texel_world. light_view is the cascade's
// light view matrix (already built around the slice center).
void shadow_bounds_fit(shadow_cascade *cascade,
                       const shadow_corners *fc,
                       mat4 light_view,
                       int map_size);

// snap an ortho box origin to the shadow texel grid so a sub-texel camera
// nudge doesnt shift every shadow by a fraction of a texel (the shimmer).
// returns the snapped min corner; texel is bounds_world / map_size.
vec3 shadow_bounds_snap(vec3 box_min, float texel);

#endif
