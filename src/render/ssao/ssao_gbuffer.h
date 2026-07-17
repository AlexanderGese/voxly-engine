#ifndef RENDER_SSAO_GBUFFER_H
#define RENDER_SSAO_GBUFFER_H
#include "../gl.h"
#include "../../math/vec3.h"
#include "../../math/mat4.h"
// the inputs ssao needs from the main geometry pass. we dont own these
// textures — the deferred/geometry stage does — we just borrow handles.
//
// view-space position can be reconstructed from depth + the inverse proj,
// which is what the gpu path does to save a render target. but if the
// geometry pass already wrote a position buffer we can take it directly.
typedef struct {
    glid tex_depth;     // depth buffer (or linear depth), required
    glid tex_normal;    // view-space normals, required
    glid tex_position;  // optional view-space position; 0 -> reconstruct

    mat4 proj;          // projection used for the geometry pass
    mat4 inv_proj;      // its inverse, for depth->view reconstruction
    int  w, h;          // full-res dimensions of the gbuffer
} ssaox_gbuffer;
// zero-init helper. sets identity matrices and null handles.
void ssaox_gbuffer_clear(ssaox_gbuffer *g);
// returns 1 if the gbuffer has the minimum set of inputs to run.
int  ssaox_gbuffer_valid(const ssaox_gbuffer *g);
// reconstruct a view-space position from a uv + sampled ndc depth using the
// inverse projection. mirrors the glsl reconstruction so the cpu reference
// matches. uv in [0,1], depth in [0,1].
vec3 ssaox_gbuffer_view_pos(const ssaox_gbuffer *g, float u, float v, float depth);
#endif
