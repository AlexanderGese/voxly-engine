#ifndef RENDER_TEXT_CTX_H
#define RENDER_TEXT_CTX_H

// the front door. owns the font, the batch, and the shader program. hud code
// pokes this and nothing else: begin a frame, draw some strings, end the frame.
// internally it lays out, batches, and flushes. keeps the call sites tidy.

#include "text_font.h"
#include "text_batch.h"
#include "text_layout.h"
#include "text_common.h"

typedef struct {
    text_font  font;
    text_batch batch;
    glid       prog;
    int        ready;

    // default options applied by the convenience draw_* calls. callers wanting
    // wrapping/alignment go through text_ctx_draw_ex with their own opts.
    text_rgba  default_color;
    int        shadow;        // draw drop shadows by default
} text_ctx;

// loads shader, bakes builtin font, sets up the batch. returns 1 on success.
int  text_ctx_init(text_ctx *c);
void text_ctx_destroy(text_ctx *c);

// frame bracket. begin sets the screen size for the ortho projection and clears
// the batch; end flushes everything in one draw call.
void text_ctx_begin(text_ctx *c, int screen_w, int screen_h);
void text_ctx_end(text_ctx *c);

// simplest call: one line (or '\n'-separated lines) at a pixel position, using
// the default color/shadow. returns the advance width of the longest line.
float text_ctx_draw(text_ctx *c, const char *s, float x, float y);

// colored variant.
float text_ctx_draw_color(text_ctx *c, const char *s, float x, float y,
                          text_rgba color);

// full control: your own layout opts + color + per-call shadow toggle.
// returns the laid-out block size via out_w/out_h (either may be NULL).
void text_ctx_draw_ex(text_ctx *c, const char *s, float x, float y,
                      const text_layout_opts *opts, text_rgba color,
                      int shadow, float *out_w, float *out_h);

// printf-style. formats into a small stack buffer then draws. truncates long
// output rather than allocating; it's hud text, keep it short.
float text_ctx_printf(text_ctx *c, float x, float y, const char *fmt, ...);

// measure without drawing, using the default (no-wrap) opts.
void  text_ctx_measure(text_ctx *c, const char *s, float *out_w, float *out_h);

// line height of the active font, for callers stacking rows by hand.
float text_ctx_line_height(const text_ctx *c);

#endif
