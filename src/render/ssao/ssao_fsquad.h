#ifndef RENDER_SSAO_FSQUAD_H
#define RENDER_SSAO_FSQUAD_H
#include "../gl.h"
// the obligatory fullscreen triangle. bigger than the screen on purpose so a
// single tri covers the viewport with no diagonal seam and better quad
// utilization than two tris. every post stage in here reuses one of these.
typedef struct {
    glid vao;
    glid vbo;
} ssaox_fsquad;
int  ssaox_fsquad_init(ssaox_fsquad *q);
void ssaox_fsquad_destroy(ssaox_fsquad *q);
void ssaox_fsquad_draw(const ssaox_fsquad *q);
#endif
