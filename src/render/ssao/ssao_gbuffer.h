#ifndef RENDER_SSAO_GBUFFER_H
#define RENDER_SSAO_GBUFFER_H
#include "../gl.h"
#include "../../math/vec3.h"
#include "../../math/mat4.h"
typedef struct {
    glid tex_depth;     // depth buffer (or linear depth), required
    glid tex_normal;    // view-space normals, required
    glid tex_position;  // optional view-space position; 0 -> reconstruct

    mat4 proj;          // projection used for the geometry pass
    mat4 inv_proj;      // its inverse, for depth->view reconstruction
    int  w, h;          // full-res dimensions of the gbuffer
} ssaox_gbuffer;
#endif
