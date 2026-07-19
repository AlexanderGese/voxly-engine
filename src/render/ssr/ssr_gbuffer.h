#ifndef RENDER_SSR_GBUFFER_H
#define RENDER_SSR_GBUFFER_H

#include "../gl.h"
#include "../../math/vec2.h"
#include "../../math/vec3.h"
#include "../../math/mat4.h"

// the inputs ssr borrows from the main geometry/lighting pass. we dont own
// these textures — the deferred stage does — we just hold the handles.
//
// the march works entirely in view space, so we need depth (to reconstruct
// view positions), view-space normals (to build the reflected ray), and the
// lit color buffer (what we actually sample reflections out of). the material
// buffer is optional; without it we treat the whole scene as one roughness.

typedef struct {
    glid tex_depth;     // depth buffer (ndc depth in .r), required
    glid tex_normal;    // view-space normals, required
    glid tex_color;     // lit scene color, required (we reflect this)
    glid tex_material;  // optional roughness(.r)/metalness(.g); 0 -> default

    mat4 proj;          // projection used for the geometry pass
    mat4 inv_proj;      // its inverse, depth->view reconstruction
    int  w, h;          // full-res dimensions of the gbuffer
} ssrx_gbuffer;

// zero-init helper. identity matrices, null handles.
void ssrx_gbuffer_clear(ssrx_gbuffer *g);

// returns 1 if the gbuffer has the minimum inputs to run a reflection pass.
int  ssrx_gbuffer_valid(const ssrx_gbuffer *g);

// reconstruct a view-space position from a uv + sampled ndc depth via the
// inverse projection. mirrors the glsl reconstruction so the cpu march matches
// the shader. uv in [0,1], depth in [0,1].
vec3 ssrx_gbuffer_view_pos(const ssrx_gbuffer *g, float u, float v, float depth);

// project a view-space point back to screen uv + ndc depth. returns 0 if the
// point is at/behind the near plane (w <= 0), in which case out_* are left
// untouched. the inverse of view_pos, basically.
int  ssrx_gbuffer_project(const ssrx_gbuffer *g, vec3 view_pos,
                          vec2 *out_uv, float *out_depth);

#endif
