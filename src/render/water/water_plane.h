#ifndef RENDER_WATER_WATER_PLANE_H
#define RENDER_WATER_WATER_PLANE_H

#include "../../math/vec3.h"
#include "../../math/vec4.h"
#include "../../math/plane.h"

// the flat surface a body of water lives on. for a voxel world all the water
// in a given pool shares one height (sea level usually) so we model it as a
// single horizontal plane and clip the world against it.

typedef struct {
    float height;       // world y of the surface
    plane up;           // normal +y, used when rendering refraction (below)
    plane down;         // normal -y, used when rendering reflection (above)
} water_plane;

// build a surface at world height y. the bias from water_config nudges the
// clip planes so we dont z-fight the real geometry.
water_plane water_plane_make(float y);

// is a point above the surface? ties (exactly on the plane) count as above.
int water_plane_above(const water_plane *wp, vec3 p);

// reflect a world-space point across the surface. used to mirror the camera
// for the reflection pass.
vec3 water_plane_reflect_point(const water_plane *wp, vec3 p);

// reflect a direction across the surface (just flips y). normalized in == out.
vec3 water_plane_reflect_dir(const water_plane *wp, vec3 d);

// the clip plane equation (a,b,c,d) for the given pass, in world space.
// pass_reflection==1 -> keep geometry above the surface,
// pass_reflection==0 -> keep geometry below (refraction).
vec4 water_plane_clip_eq(const water_plane *wp, int pass_reflection);

#endif
