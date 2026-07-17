#ifndef RENDER_SSAO_PASS_H
#define RENDER_SSAO_PASS_H
#include "ssao_config.h"
#include "ssao_kernel.h"
#include "ssao_noise.h"
#include "ssao_blur.h"
#include "ssao_fsquad.h"
#include "ssao_gbuffer.h"
// the gpu ssao pass. owns the occlusion fbo, the blur stage, the kernel +
// noise, and the two shaders. you feed it a gbuffer each frame and it leaves
// the final (blurred) occlusion in ssaox_pass_result().
//
// the occlusion buffer runs at full_res / scale. result is single channel,
// 1.0 = fully lit, lower = more occluded (so the lighting shader can just
// multiply ambient by it).
typedef struct {
    // owned gl objects
    glid fbo_occl;
    glid tex_occl;        // raw occlusion, R8, before blur
    glid prog_occl;       // ssaox_occlude.frag
    glid prog_blur;       // ssaox_blur.frag (shared into the blur stage)

    ssaox_blur   blur;
    ssaox_fsquad quad;
    ssaox_kernel kernel;
    ssaox_noise  noise;

    int   full_w, full_h; // gbuffer res we were last sized for
    int   w, h;           // occlusion res (full / scale)
    int   scale;          // downsample factor

    // tunables, mutated freely at runtime
    float radius;
    float bias;
    float power;
    float strength;

    int   enabled;        // 0 if shaders failed to load — pass is a no-op
} ssaox_pass;
// init at a given gbuffer resolution. clamps kernel to count, builds noise.
// returns 1 on success, 0 if the pass is disabled (still safe to call run).
int  ssaox_pass_init(ssaox_pass *p, int full_w, int full_h, int kernel_count);
void ssaox_pass_destroy(ssaox_pass *p);
// resize to a new gbuffer resolution. cheap no-op if unchanged.
void ssaox_pass_resize(ssaox_pass *p, int full_w, int full_h);
// run the full pass against `g`. binds its own fbo, draws occlusion, blurs.
// leaves gl state with fbo 0 bound. does nothing useful if !enabled or the
// gbuffer is invalid (returns 0 in that case).
int  ssaox_pass_run(ssaox_pass *p, const ssaox_gbuffer *g);
// the texture the lighting stage should sample (blurred occlusion).
glid ssaox_pass_result(const ssaox_pass *p);
// re-scatter the kernel (e.g. on a "reseed" debug key) and re-pack it.
void ssaox_pass_reseed(ssaox_pass *p, uint64_t seed);
#endif
