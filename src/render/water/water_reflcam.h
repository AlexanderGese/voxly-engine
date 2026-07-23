#ifndef RENDER_WATER_WATER_REFLCAM_H
#define RENDER_WATER_WATER_REFLCAM_H

#include "../../math/mat4.h"
#include "../../math/vec4.h"
#include "../camera.h"
#include "water_plane.h"

// derives the view/proj matrices for the reflection and refraction passes.
// reflection mirrors the camera under the surface; refraction reuses the real
// camera but with an oblique-clipped projection so nothing above the water
// leaks into the refraction texture.

typedef struct {
    mat4 view;
    mat4 proj;
    vec4 clip_eq;       // world-space clip plane fed to the world shader
} water_view;

// reflection: flip the camera across the surface and invert pitch.
water_view water_reflcam_reflection(const camera *cam, const water_plane *wp);

// refraction: same camera, just attach the clip plane keeping geometry below.
water_view water_reflcam_refraction(const camera *cam, const water_plane *wp);

// modify a projection matrix so its near plane coincides with the given
// clip plane (camera/eye space). this is the lengyel oblique near-plane trick;
// it keeps the depth buffer sane while clipping at the water line.
mat4 water_reflcam_oblique(mat4 proj, vec4 clip_plane_eye);

// transform a world-space clip plane into eye space for the oblique trick.
vec4 water_reflcam_plane_to_eye(mat4 view, vec4 plane_world);

#endif
