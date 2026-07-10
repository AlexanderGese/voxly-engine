#ifndef RENDER_INSTANCING_TRANSFORM_H
#define RENDER_INSTANCING_TRANSFORM_H

#include "instance_types.h"

// turns an instance_desc into the gpu payload (the baked 4x4 + tint), and
// helps the cull pass figure out where the thing actually sits in space.
//
// the matrix is translate * rotate_y * scale, column-major to match the
// engine's mat4 convention. we don't go through mat4_mul three times per
// instance because that's a lot of float churn when you've got 50k of
// them — instead we write the 16 floats directly. it's the same result,
// just inlined. measured it once, it mattered.

// bake one instance into its gpu payload. safe to call every frame.
void instancing_transform_bake(const instance_desc *d, instance_gpu *out);

// world-space center of the instance. for a y-rotation about the origin
// the translation *is* the center (props are authored origin-centered),
// so this is just the position — but kept as a function so callers don't
// bake assumptions in.
vec3 instancing_transform_center(const instance_desc *d);

// world-space aabb, conservative. we don't rotate the box, we just take
// the bounding sphere and make a cube of it. good enough for frustum
// reject and cheaper than transforming 8 corners.
aabb instancing_transform_bounds(const instance_desc *d);

// build the full mat4 the slow-but-obvious way. used by tests and by the
// debug overlay; the hot path uses _bake which writes floats directly.
mat4 instancing_transform_matrix(const instance_desc *d);

#endif
