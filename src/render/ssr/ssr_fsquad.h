#ifndef RENDER_SSR_FSQUAD_H
#define RENDER_SSR_FSQUAD_H

#include "../gl.h"

// the obligatory fullscreen-triangle. one oversized tri covers the viewport
// with a single draw and no diagonal seam. the post_passthrough.vert derives
// uv = pos*0.5+0.5 from clip position, so we only ship the 2d positions.
//
// yes ssao has its own copy of this. no i'm not going to make a shared one for
// six floats and twenty lines — the include graph is messier than the dup.

typedef struct {
    glid vao;
    glid vbo;
} ssrx_fsquad;

int  ssrx_fsquad_init(ssrx_fsquad *q);
void ssrx_fsquad_destroy(ssrx_fsquad *q);
void ssrx_fsquad_draw(const ssrx_fsquad *q);

#endif
