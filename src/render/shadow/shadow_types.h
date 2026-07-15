#ifndef RENDER_SHADOW_TYPES_H
#define RENDER_SHADOW_TYPES_H
#include "shadow_config.h"
#include "../gl.h"
#include "../../math/vec3.h"
#include "../../math/mat4.h"
#include "../../math/aabb.h"
// shared structs for the csm subsystem. kept separate so split/bounds/matrix
// can all see them without dragging in the gl-heavy shadow_map header.
// one cascade slice. split distances are along the camera view axis (positive,
// in world units). the matrices are filled per frame by shadow_matrix.
typedef struct {
    float near_d;          // near split distance (view space, +)
    float far_d;           // far split distance
    mat4  view;            // light view matrix
    mat4  proj;            // light ortho projection
    mat4  view_proj;       // proj * view, cached for the depth pass + sampling
    aabb  bounds;          // ortho box in light space (for debug / culling)
    float texel_world;     // world size of one shadow texel, for snapping/bias
} shadow_cascade;
// the eight world-space corners of a (sub)frustum. ordered near0..3, far0..3.
typedef struct {
    vec3 c[8];
} shadow_corners;
// top level csm state. owns nothing gl here — shadow_map owns the textures.
typedef struct {
    shadow_cascade cascade[SHADOW_CASCADE_COUNT];
    float          splits[SHADOW_CASCADE_COUNT + 1]; // splits[0]=near .. splits[n]=far
    vec3           light_dir;     // normalized, points *from* light toward scene
    int            count;         // == SHADOW_CASCADE_COUNT, kept for loops
    int            enabled;
} shadow_csm;
#endif
