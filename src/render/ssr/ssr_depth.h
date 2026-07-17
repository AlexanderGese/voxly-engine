#ifndef RENDER_SSR_DEPTH_H
#define RENDER_SSR_DEPTH_H
#include "ssr_gbuffer.h"
#include "../../math/vec2.h"
// a cpu-side view of a depth buffer for the reference march. the gpu path
// samples a gl texture; the tests and the cpu march need a plain float array.
// this wraps one so the march can ask "what's the scene view-z under this uv"
// without caring where the depth came from.
//
// the depth values are ndc depth in [0,1] just like glReadPixels / the texture
// would hand back. we reconstruct view-z on demand through the gbuffer's
// inverse projection so it stays consistent with the shader.
typedef struct {
    const float *ndc;   // w*h ndc depth values, row major, [0,1]
    int w, h;
} ssrx_depthbuf;
// nearest-neighbour fetch of ndc depth at a uv. clamps to the edge texel. uvs
// outside [0,1] are clamped, not wrapped (mirrors GL_CLAMP_TO_EDGE).
float ssrx_depth_sample_ndc(const ssrx_depthbuf *d, vec2 uv);
// bilinear ndc depth. smoother for the refine step where small steps matter.
float ssrx_depth_sample_ndc_bilinear(const ssrx_depthbuf *d, vec2 uv);
int ssrx_depth_scene_view_z(const ssrx_depthbuf *d, const ssrx_gbuffer *g,
                            vec2 uv, float *out_view_z);
#endif
