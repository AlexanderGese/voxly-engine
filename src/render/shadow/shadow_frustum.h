#ifndef RENDER_SHADOW_FRUSTUM_H
#define RENDER_SHADOW_FRUSTUM_H

#include "shadow_types.h"

// extract the world-space corners of a slice of the camera frustum. we need
// these to fit each cascade's light box tightly around exactly the geometry
// that cascade is responsible for.

// build the 8 corners of the sub-frustum bounded by [near_d, far_d] along the
// view axis. cam_view/cam_proj are the *camera's* matrices (proj should be the
// full perspective; we re-slice it with the near/far args, not its own planes).
void shadow_frustum_corners(shadow_corners *out,
                            mat4 cam_view, float fov_rad, float aspect,
                            float near_d, float far_d);

// centroid of the 8 corners. used to anchor the light view.
vec3 shadow_frustum_center(const shadow_corners *fc);

// radius of the bounding sphere around the corners (max dist from center).
// fitting the ortho box to a sphere keeps its size constant as the camera
// rotates, which is what stops the shadow edges from shimmering.
float shadow_frustum_radius(const shadow_corners *fc, vec3 center);

#endif
