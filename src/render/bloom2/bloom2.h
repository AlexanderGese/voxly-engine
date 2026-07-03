#ifndef RENDER_BLOOM2_H
#define RENDER_BLOOM2_H
#include "../gl.h"
#include "bloom2_chain.h"
#include "bloom2_programs.h"
#include "bloom2_params.h"
#include "bloom2_quad.h"
#include "bloom2_tint.h"
typedef struct {
    bloom2_chain    chain;
    bloom2_programs prog;
    bloom2_params   params;
    bloom2_quad     quad;
    bloom2_tint     tint;
    int  ready;          // 1 if shaders + chain are good to go
    unsigned frames;     // ran-frame counter, mostly for debug
} bloom2;
int  bloom2_init(bloom2 *b, int w, int h);
void bloom2_destroy(bloom2 *b);
void bloom2_resize(bloom2 *b, int w, int h);
void bloom2_run(bloom2 *b, glid scene_tex, glid dst, int dst_w, int dst_h);
bloom2_params *bloom2_get_params(bloom2 *b);
#endif
