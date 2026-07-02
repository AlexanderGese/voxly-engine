#ifndef RENDER_FONT_H
#define RENDER_FONT_H

// tiny fixed-width bitmap font. ascii 32..126. embedded as byte arrays.
// 6x8 glyphs. good enough for hud text.

#include "gl.h"

typedef struct {
    glid tex;
    int  glyph_w;
    int  glyph_h;
    int  atlas_w;
    int  atlas_h;
} font;

int  font_init(font *f);
void font_destroy(font *f);
void font_measure(const font *f, const char *s, int *out_w, int *out_h);

// returns number of quads written into `out_verts`
// each quad = 6 vertices of (px,py,u,v)
int  font_build_text(const font *f, const char *s, int x, int y,
                     float *out_verts, int max_verts);

#endif
