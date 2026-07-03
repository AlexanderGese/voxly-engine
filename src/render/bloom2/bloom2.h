#ifndef RENDER_BLOOM2_H
#define RENDER_BLOOM2_H

#include "../gl.h"
#include "bloom2_chain.h"
#include "bloom2_programs.h"
#include "bloom2_params.h"
#include "bloom2_quad.h"
#include "bloom2_tint.h"

// public face of the bloom2 post chain. the rest of the renderer only needs
// this header. usage:
//
// bloom2 b;
// bloom2_init(&b, win_w, win_h);
// ... per frame, after the hdr scene is rendered to scene_tex ...
// bloom2_run(&b, scene_tex, /*dst fbo*/ 0, win_w, win_h);
// ... on resize ...
// bloom2_resize(&b, new_w, new_h);
// bloom2_destroy(&b);
//
// bloom2_run additively composites the glow over whatever framebuffer is
// bound, so call it after you've blitted/tonemapped the scene there.

typedef struct {
    bloom2_chain    chain;
    bloom2_programs prog;
    bloom2_params   params;
    bloom2_quad     quad;
    bloom2_tint     tint;
    int  ready;          // 1 if shaders + chain are good to go
    unsigned frames;     // ran-frame counter, mostly for debug
} bloom2;

// build everything for a w x h backbuffer. returns 1 on success. on failure
// (shaders missing etc) it returns 0 but leaves the struct safe to destroy
// and to call run() on (run becomes a no-op).
int  bloom2_init(bloom2 *b, int w, int h);
void bloom2_destroy(bloom2 *b);

// rebuild the mip chain for a new size. cheap to call every frame; it early
// outs if the size hasnt changed.
void bloom2_resize(bloom2 *b, int w, int h);

// run the full chain: bright -> downsample -> upsample -> composite. dst is
// the framebuffer to composite into (0 = backbuffer). does nothing if bloom
// is disabled or not ready.
void bloom2_run(bloom2 *b, glid scene_tex, glid dst, int dst_w, int dst_h);

// grab a mutable pointer to the live params so the debug ui can tweak them.
bloom2_params *bloom2_get_params(bloom2 *b);

#endif
