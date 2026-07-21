#include "text_label.h"

#include <string.h>

// fnv-1a 64. small, fast, good enough for change detection.
static uint64_t fnv1a(const char *s) {
    uint64_t h = 1469598103934665603ull;
    for (const unsigned char *p = (const unsigned char*)s; *p; p++) {
        h ^= (uint64_t)*p;
        h *= 1099511628211ull;
    }
    return h;
}

// fold the opts into the hash so a change in wrap width / alignment also forces
// a reshape. quick and dirty: mix the bytes in.
static uint64_t mix_opts(uint64_t h, const text_layout_opts *o) {
    const unsigned char *b = (const unsigned char*)o;
    for (size_t i = 0; i < sizeof *o; i++) {
        h ^= b[i];
        h *= 1099511628211ull;
    }
    return h;
}

void text_label_init(text_label *lb) {
    memset(lb, 0, sizeof *lb);
}

void text_label_free(text_label *lb) {
    if (lb->valid) text_layout_free(&lb->layout);
    memset(lb, 0, sizeof *lb);
}

void text_label_invalidate(text_label *lb) {
    lb->valid = 0;
    lb->hash  = 0;
}

int text_label_set(text_label *lb, const text_font *font, const char *s,
                   const text_layout_opts *opts) {
    if (!s) s = "";
    uint64_t h = mix_opts(fnv1a(s), opts);

    if (lb->valid && h == lb->hash) {
        // nothing changed, keep the cached layout
        return 0;
    }

    if (lb->valid) text_layout_free(&lb->layout);
    lb->valid = 0;

    if (!text_layout_run(font, s, opts, &lb->layout)) {
        // shaping failed; leave the label empty but remember the hash so we
        // dont spin retrying every frame on the same broken input.
        lb->hash   = h;
        lb->width  = 0;
        lb->height = 0;
        return 1;
    }

    lb->opts   = *opts;
    lb->hash   = h;
    lb->valid  = 1;
    lb->width  = lb->layout.width;
    lb->height = lb->layout.height;
    return 1;
}

void text_label_draw(const text_label *lb, text_batch *b, const text_font *font,
                     float x, float y, text_rgba color) {
    if (!lb->valid) return;
    text_batch_push_layout(b, font, &lb->layout, x, y, color);
}

void text_label_draw_shadow(const text_label *lb, text_batch *b,
                            const text_font *font, float x, float y,
                            text_rgba color, text_rgba shadow) {
    if (!lb->valid) return;
    // 2px offset matches the live-draw shadow in text_ctx
    text_batch_push_layout_shadow(b, font, &lb->layout, x, y,
                                  color, shadow, 2.0f, 2.0f);
}
