#ifndef RENDER_SHADOW_MATRIX_H
#define RENDER_SHADOW_MATRIX_H

#include "shadow_types.h"

// build the per-cascade light view + ortho projection. this is where split,
// frustum and bounds all come together each frame.

// (re)compute every cascade's view/proj/view_proj for the current camera and
// sun direction. csm->splits must already be filled by shadow_split_compute.
// fov/aspect are the camera's; light_dir points from the sun into the scene.
void shadow_matrix_update(shadow_csm *csm,
                          mat4 cam_view, float fov_rad, float aspect,
                          vec3 light_dir);

// the bias matrix that remaps clip-space [-1,1] to texture [0,1]. handy to
// hand the shader a single matrix that goes world -> shadow uv directly.
mat4 shadow_matrix_bias(void);

// world -> shadow-uv matrix for cascade i (bias * view_proj). this is what the
// main lighting shader samples with.
mat4 shadow_matrix_sample(const shadow_csm *csm, int cascade);

// clamp every cascade's ortho near/far z to the scene's actual extent so we
// dont waste depth precision on empty sky/void. call after shadow_matrix_update
// with the world aabb of the loaded chunks. rebuilds proj + view_proj.
void shadow_matrix_clamp_scene(shadow_csm *csm, aabb scene_world);

#endif
