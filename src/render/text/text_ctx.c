#include "text_ctx.h"
#include "../../util/log.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// where the text shader lives. matches the project's shaders/ convention (see
// the old render/text.c which loaded shaders/text.vert + .frag).
#define TEXT_VERT_PATH "shaders/text2d.vert"
#define TEXT_FRAG_PATH "shaders/text2d.frag"

int text_ctx_init(text_ctx *c) {
    memset(c, 0, sizeof *c);

    c->prog = gl_load_shader(TEXT_VERT_PATH, TEXT_FRAG_PATH);
    if (!c->prog) {
        LOGE("text_ctx: shader load failed");
        return 0;
    }
    if (!text_font_init_builtin(&c->font)) {
        LOGE("text_ctx: font bake failed");
        gl_delete_shader(c->prog);
        return 0;
    }
    if (!text_batch_init(&c->batch, c->prog)) {
        LOGE("text_ctx: batch init failed");
        text_font_destroy(&c->font);
        gl_delete_shader(c->prog);
        return 0;
    }

    c->default_color = TEXT_WHITE;
    c->shadow = 1;
    c->ready = 1;
    return 1;
}

void text_ctx_destroy(text_ctx *c) {
    if (!c->ready) return;
    text_batch_destroy(&c->batch);
    text_font_destroy(&c->font);
    gl_delete_shader(c->prog);
    memset(c, 0, sizeof *c);
}

void text_ctx_begin(text_ctx *c, int screen_w, int screen_h) {
    if (!c->ready) return;
    text_batch_set_screen(&c->batch, (float)screen_w, (float)screen_h);
    text_batch_reset(&c->batch);
}

void text_ctx_end(text_ctx *c) {
    if (!c->ready) return;
    text_batch_flush(&c->batch, c->font.atlas.tex);
}

// the default opts: single block, left aligned, no wrap.
static text_layout_opts default_opts(void) {
    text_layout_opts o;
    memset(&o, 0, sizeof o);
    o.max_width    = 0;
    o.align        = TEXT_ALIGN_LEFT;
    o.line_spacing = 1.0f;
    o.tab_cols     = 0;
    return o;
}

void text_ctx_draw_ex(text_ctx *c, const char *s, float x, float y,
                      const text_layout_opts *opts, text_rgba color,
                      int shadow, float *out_w, float *out_h) {
    if (!c->ready || !s) {
        if (out_w) *out_w = 0;
        if (out_h) *out_h = 0;
        return;
    }
    text_layout l;
    if (!text_layout_run(&c->font, s, opts, &l)) {
        if (out_w) *out_w = 0;
        if (out_h) *out_h = 0;
        return;
    }

    if (shadow) {
        float d = 1.0f * 2.0f; // shadow offset tracks the font scale-ish
        text_batch_push_layout_shadow(&c->batch, &c->font, &l, x, y,
                                      color, TEXT_SHADOW, d, d);
    } else {
        text_batch_push_layout(&c->batch, &c->font, &l, x, y, color);
    }

    if (out_w) *out_w = l.width;
    if (out_h) *out_h = l.height;
    text_layout_free(&l);
}

float text_ctx_draw_color(text_ctx *c, const char *s, float x, float y,
                          text_rgba color) {
    text_layout_opts o = default_opts();
    float w = 0;
    text_ctx_draw_ex(c, s, x, y, &o, color, c->shadow, &w, NULL);
    return w;
}

float text_ctx_draw(text_ctx *c, const char *s, float x, float y) {
    return text_ctx_draw_color(c, s, x, y, c->default_color);
}

float text_ctx_printf(text_ctx *c, float x, float y, const char *fmt, ...) {
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    if (n < 0) return 0;
    // vsnprintf already null-terminates and truncates; nothing else to do.
    return text_ctx_draw(c, buf, x, y);
}

void text_ctx_measure(text_ctx *c, const char *s, float *out_w, float *out_h) {
    text_layout_opts o = default_opts();
    text_layout_measure(&c->font, s, &o, out_w, out_h);
}

float text_ctx_line_height(const text_ctx *c) {
    return (float)c->font.line_height;
}
