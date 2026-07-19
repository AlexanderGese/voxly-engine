#ifndef RENDER_TEXT_LABEL_H
#define RENDER_TEXT_LABEL_H
#include "text_font.h"
#include "text_layout.h"
#include "text_batch.h"
#include "text_common.h"
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
int  text_label_set(text_label *lb, const text_font *font, const char *s,
                    const text_layout_opts *opts);
void text_label_invalidate(text_label *lb);
#endif
