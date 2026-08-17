#ifndef UI_HUD2_BATCH_H
#define UI_HUD2_BATCH_H
#include "../../render/gl.h"
#include "hud2_color.h"
// a tiny immediate-mode-ish 2d batcher for the hud. everything the hud draws
// (rects, lines, glyph quads) gets pushed into one big vertex buffer and
// flushed once per frame. way fewer draw calls than the old hud.c which
// re-uploaded per quad. coordinates are in pixels, origin top-left.
//
// vertex layout is interleaved: x, y, r, g, b, a  (6 floats). the shader is
// expected to map pixel coords to ndc via u_sw / u_sh uniforms.
#define HUD2_BATCH_MAX_VERTS  (4096 * 6)
#define HUD2_FLOATS_PER_VERT  6
typedef struct {
    glid  vao;
    glid  vbo;
    glid  prog;
    int   ready;

    float verts[HUD2_BATCH_MAX_VERTS];
    int   count;        // number of verts pushed (not floats)

    int   sw, sh;       // screen size for the current frame
    int   draws;        // stat: flushes this frame
} hud2_batch;
void hud2_batch_init(hud2_batch *b, glid prog);
void hud2_batch_destroy(hud2_batch *b);
// begin a frame. resets the cursor and records screen size.
void hud2_batch_begin(hud2_batch *b, int sw, int sh);
// push primitives. these auto-flush if the buffer would overflow.
void hud2_batch_quad(hud2_batch *b, float x, float y, float w, float h,
                     hud2_color c);
void hud2_batch_rect_outline(hud2_batch *b, float x, float y, float w, float h,
                             float thick, hud2_color c);
void hud2_batch_tri(hud2_batch *b,
                    float x0, float y0, float x1, float y1,
                    float x2, float y2, hud2_color c);
void hud2_batch_line(hud2_batch *b, float x0, float y0, float x1, float y1,
                     float thick, hud2_color c);
// flush whatever is buffered to the gpu and draw it.
void hud2_batch_flush(hud2_batch *b);
// end a frame: flushes and restores gl state.
void hud2_batch_end(hud2_batch *b);
#endif
