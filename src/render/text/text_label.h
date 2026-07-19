#ifndef RENDER_TEXT_LABEL_H
#define RENDER_TEXT_LABEL_H

// a cached, pre-laid-out string. most hud text doesnt change every frame (the
// "version" string, key hints, the pause menu) so re-shaping it 60x a second is
// daft. a label holds the layout result and only re-runs it when the source
// string or options actually change. push it into a batch each frame for ~free.

#include "text_font.h"
#include "text_layout.h"
#include "text_batch.h"
#include "text_common.h"

// fnv-1a over the source string so we can cheaply detect changes without
// strcmp'ing a possibly-long string every frame. collisions are theoretically
// possible but for hud strings it's fine; worst case we skip one relayout.
typedef struct {
    text_layout      layout;
    text_layout_opts opts;
    uint64_t         hash;     // hash of the last shaped string
    int              valid;
    float            width;
    float            height;
} text_label;

void text_label_init(text_label *lb);
void text_label_free(text_label *lb);

// shape `s` into the label if it changed (or opts changed). cheap no-op if the
// content is identical to last call. returns 1 if it re-shaped this call.
int  text_label_set(text_label *lb, const text_font *font, const char *s,
                    const text_layout_opts *opts);

// force the next set() to reshape even if the string matches (e.g. after the
// font was re-baked at a new scale).
void text_label_invalidate(text_label *lb);

// queue the cached glyphs into a batch at (x,y).
void text_label_draw(const text_label *lb, text_batch *b, const text_font *font,
                     float x, float y, text_rgba color);

void text_label_draw_shadow(const text_label *lb, text_batch *b,
                            const text_font *font, float x, float y,
                            text_rgba color, text_rgba shadow);

#endif
