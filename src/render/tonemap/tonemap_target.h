#ifndef RENDER_TONEMAP_TARGET_H
#define RENDER_TONEMAP_TARGET_H

#include "../gl.h"

// optional ldr output target. usually the tonemap pass writes straight to the
// backbuffer (fbo 0), but if something downstream (fxaa, ui compositing) wants
// the graded ldr image as a texture, we render into this instead.

typedef struct {
    glid fbo;
    glid tex;
    int  w, h;
} tonemap_target;

// create an 8-bit rgba target. returns 0 on failure (struct left zeroed).
int  tonemap_target_create(tonemap_target *t, int w, int h);
void tonemap_target_destroy(tonemap_target *t);

// resize in place; no-op if the size is unchanged. returns 1 on success.
int  tonemap_target_resize(tonemap_target *t, int w, int h);

// bind for rendering, set viewport. pass NULL to bind the backbuffer.
void tonemap_target_bind(const tonemap_target *t);

// bind the color tex to a sampler unit; returns the unit.
int  tonemap_target_bind_tex(const tonemap_target *t, int unit);

#endif
