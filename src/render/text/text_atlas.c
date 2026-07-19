#include "text_atlas.h"
#include "../../util/log.h"
#include <stdlib.h>
#include <string.h>
#define ATLAS_PAD 1
int text_atlas_init(text_atlas *a, int w, int h) {
    memset(a, 0, sizeof *a);
    if (w <= 0 || h <= 0) return 0;
    a->pixels = calloc((size_t)w * h * 4, 1);
    if (!a->pixels) { LOGE("text_atlas: alloc %dx%d failed", w, h); return 0; }
    a->w = w;
    a->h = h;
    a->shelf_x = ATLAS_PAD;
    a->shelf_y = ATLAS_PAD;
    a->shelf_h = 0;
    a->dirty = 1;
    return 1;
}

void text_atlas_destroy(text_atlas *a) {
    if (a->tex) glDeleteTextures(1, &a->tex);
free(a->pixels);
memset(a, 0, sizeof *a);
}

int text_atlas_pack(text_atlas *a, int w, int h, text_atlas_rect *out) {
    if (w <= 0 || h <= 0) { *out = (text_atlas_rect){0, 0, 0, 0}; return 1; }
    if (w > a->w - 2 * ATLAS_PAD) return 0;  // never going to fit, anywhere

    // doesnt fit in the current shelf horizontally? drop to a new one.
    if (a->shelf_x + w + ATLAS_PAD > a->w) {
        a->shelf_y += a->shelf_h + ATLAS_PAD;
        a->shelf_x  = ATLAS_PAD;
        a->shelf_h  = 0;
    }
    // out of vertical room. caller has to make a bigger atlas, not our problem.
    if (a->shelf_y + h + ATLAS_PAD > a->h) return 0;

    out->x = a->shelf_x;
    out->y = a->shelf_y;
    out->w = w;
    out->h = h;

    a->shelf_x += w + ATLAS_PAD;
    if (h > a->shelf_h) a->shelf_h = h;
    return 1;
}

void text_atlas_blit_a8(text_atlas *a, const text_atlas_rect *r,
                        const uint8_t *src, int src_stride) {
    for (int row = 0;
row < r->h;
glBindTexture(GL_TEXTURE_2D, a->tex);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, a->w, a->h, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, a->pixels);
a->dirty = 0;
return;
}
    if (!a->dirty) return;
glBindTexture(GL_TEXTURE_2D, a->tex);
glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, a->w, a->h,
                    GL_RGBA, GL_UNSIGNED_BYTE, a->pixels);
a->dirty = 0;
