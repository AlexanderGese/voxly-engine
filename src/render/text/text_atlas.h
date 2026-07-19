#ifndef RENDER_TEXT_ATLAS_H
#define RENDER_TEXT_ATLAS_H

// a cpu-side rgba8 glyph atlas plus the gl texture it gets uploaded into.
// we pack glyph bitmaps in with a dead-simple shelf packer (skyline-lite).
// it wastes a bit of vertical space but the packing code is 30 lines instead
// of 300 and the atlas is tiny anyway.

#include <stdint.h>
#include "text_common.h"

// a rectangle reserved inside the atlas. pixel coords, top-left origin.
typedef struct {
    int x, y;
    int w, h;
} text_atlas_rect;

typedef struct {
    uint8_t *pixels;    // w*h*4, rgba8, owned
    int      w, h;

    // shelf state. each shelf is a horizontal band; we fill left-to-right and
    // open a new band below when the current one cant fit the next glyph.
    int shelf_x;        // x cursor in the current shelf
    int shelf_y;        // top y of the current shelf
    int shelf_h;        // height of the current shelf (tallest glyph so far)

    glid tex;           // gl handle, 0 until uploaded
    int  dirty;         // pixels changed since last upload
} text_atlas;

// alloc an atlas of w*h. pixels start fully transparent. w/h should be powers
// of two-ish; we dont enforce it but the gpu prefers it.
int  text_atlas_init(text_atlas *a, int w, int h);
void text_atlas_destroy(text_atlas *a);

// reserve a w*h rect. returns 1 and fills *out on success, 0 if it doesnt fit.
int  text_atlas_pack(text_atlas *a, int w, int h, text_atlas_rect *out);

// blit a single-channel (alpha) coverage bitmap into a packed rect, expanding
// it to rgba (white rgb, src as alpha). src is row-major, src_stride bytes/row.
void text_atlas_blit_a8(text_atlas *a, const text_atlas_rect *r,
                        const uint8_t *src, int src_stride);

// blit straight rgba (used for the embedded bitmap font which is 1bpp blown up)
void text_atlas_blit_rgba(text_atlas *a, const text_atlas_rect *r,
                          const uint8_t *src, int src_stride);

// (re)upload dirty pixels to the gl texture, creating it on first call.
void text_atlas_upload(text_atlas *a);

// convert a packed rect to 0..1 uv corners. half-texel inset to dodge bleeding.
void text_atlas_uv(const text_atlas *a, const text_atlas_rect *r,
                   float *u0, float *v0, float *u1, float *v1);

#endif
