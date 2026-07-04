#ifndef RENDER_DEBUGDRAW_GL_H
#define RENDER_DEBUGDRAW_GL_H

#include "../gl.h"
#include "debugdraw_vertex.h"

// gpu side of the debug drawer. one dynamic vbo we re-upload every flush,
// one shader. nothing persistent on the gpu — the cpu owns the geometry.
// (yes, orphaning a big buffer each frame. its debug draw, perf is fine.)

typedef struct {
    glid vao;
    glid vbo;
    glid prog;
    int  vbo_bytes;     // current allocation, grown on demand
} ddgl;

int  ddgl_init(ddgl *g);
void ddgl_destroy(ddgl *g);

// push a vertex span up to the vbo, growing the allocation if needed.
void ddgl_upload(ddgl *g, const ddvert *verts, int count);

// bind shader + set the combined view*proj. call once per flush.
void ddgl_begin(ddgl *g, const float *viewproj);

// draw [first, first+count) of the uploaded buffer as `mode` (GL_LINES etc).
// depth_test 0 disables the depth test so overlays draw on top.
void ddgl_draw_range(ddgl *g, unsigned mode, int first, int count,
                     int depth_test, float line_width, float point_size);

void ddgl_end(ddgl *g);

#endif
