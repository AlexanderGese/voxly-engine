#include "text_layout.h"
#include "../../util/darray.h"

#include <string.h>

// the layout pass walks the string once, accumulating a "pen" x along the
// current line. words are buffered so that when wrapping kicks in we can shove
// the whole word down a line instead of breaking mid-word. it's a tiny state
// machine: we flush the pending word on whitespace, newline, or end of string.

typedef struct {
    const text_font *font;
    text_layout_opts opts;
    float line_h;
    int   tab_advance;       // pixels per tab stop column block

    text_placement *items;   // darray, accumulates final placements
    float pen_x;             // current pen on the active line
    int   line;              // current line index
    float max_line_w;        // widest line seen so far

    // pending word buffer: glyphs not yet committed to a line because we might
    // still wrap them. stored relative to word start (x offset within word).
    text_placement *word;    // darray
    float word_w;            // total advance width of the pending word
} layout_state;

static float tab_stop(const layout_state *st, float x) {
    // snap x up to the next multiple of tab_advance
    float n = x / (float)st->tab_advance;
    int   k = (int)n;
    if ((float)k * st->tab_advance <= x) k++;
    return (float)k * st->tab_advance;
}

// commit the buffered word onto the current line at the current pen, wrapping
// to a new line first if it wouldnt fit and the line already has content.
static void flush_word(layout_state *st) {
    if (darr_len(st->word) == 0) return;

    float wrap = st->opts.max_width;
    if (wrap > 0 && st->pen_x > 0 && st->pen_x + st->word_w > wrap) {
        // doesnt fit and we're not at line start -> new line for the word
        if (st->pen_x > st->max_line_w) st->max_line_w = st->pen_x;
        st->pen_x = 0;
        st->line++;
    }

    float base_x = st->pen_x;
    float base_y = (float)st->line * st->line_h;
    for (size_t i = 0; i < darr_len(st->word); i++) {
        text_placement p = st->word[i];
        p.x += base_x;
        p.y += base_y;
        p.line = st->line;
        darr_push(st->items, p);
    }
    st->pen_x += st->word_w;
    darr_clear(st->word);
    st->word_w = 0;
}

// append a glyph to the pending word, advancing the in-word cursor.
static void word_push_glyph(layout_state *st, const text_glyph *g,
                            uint32_t prev_cp, uint32_t cp) {
    int kern = prev_cp ? text_font_kern(st->font, prev_cp, cp) : 0;
    st->word_w += (float)kern;

    text_placement p;
    p.g    = g;
    p.x    = st->word_w + (float)g->bearing_x;
    // y: top of the glyph quad relative to line top. ascent - bearing_y puts the
    // bitmap top below the line top by however much it sits under the ascent.
    p.y    = (float)(st->font->ascent - g->bearing_y);
    p.line = 0; // patched at flush
    darr_push(st->word, p);

    st->word_w += (float)g->advance;
}

// whitespace separators get added directly to the line (they never wrap to the
// next line as leading space — we drop them at wrap points instead).
static void emit_space(layout_state *st, float advance) {
    // collapse a space that lands exactly at a wrap boundary by just advancing;
    // it contributes width but produces no quad.
    float wrap = st->opts.max_width;
    if (wrap > 0 && st->pen_x >= wrap) {
        // already overflowing, swallow the space and wrap
        if (st->pen_x > st->max_line_w) st->max_line_w = st->pen_x;
        st->pen_x = 0;
        st->line++;
        return;
    }
    st->pen_x += advance;
}

static void newline(layout_state *st) {
    flush_word(st);
    if (st->pen_x > st->max_line_w) st->max_line_w = st->pen_x;
    st->pen_x = 0;
    st->line++;
}

int text_layout_run(const text_font *font, const char *s,
                    const text_layout_opts *opts, text_layout *out) {
    memset(out, 0, sizeof *out);
    if (!font || !font->baked || !s) return 0;

    layout_state st;
    memset(&st, 0, sizeof st);
    st.font = font;
    st.opts = *opts;

    float spacing = opts->line_spacing > 0 ? opts->line_spacing : 1.0f;
    st.line_h = (float)font->line_height * spacing;

    int tab_cols = opts->tab_cols > 0 ? opts->tab_cols : font->tab_cols;
    if (tab_cols <= 0) tab_cols = 4;
    st.tab_advance = font->space_advance * tab_cols;
    if (st.tab_advance <= 0) st.tab_advance = 1;

    uint32_t prev_cp = 0;

    for (const unsigned char *p = (const unsigned char*)s; *p; p++) {
        unsigned char c = *p;

        if (c == '\n') {
            newline(&st);
            prev_cp = 0;
            continue;
        }
        if (c == '\t') {
            flush_word(&st);
            st.pen_x = tab_stop(&st, st.pen_x);
            prev_cp = 0;
            continue;
        }
        if (c == ' ') {
            flush_word(&st);
            emit_space(&st, (float)font->space_advance);
            prev_cp = 0;
            continue;
        }

        const text_glyph *g = text_font_glyph(font, c);
        word_push_glyph(&st, g, prev_cp, c);
        prev_cp = c;
    }

    // last word / line
    flush_word(&st);
    if (st.pen_x > st.max_line_w) st.max_line_w = st.pen_x;

    out->items       = st.items;
    out->count       = (int)darr_len(st.items);
    out->line_count  = st.line + 1;
    out->width       = st.max_line_w;
    out->height      = (float)out->line_count * st.line_h;
    out->line_height = st.line_h;

    darr_free(st.word);

    // alignment: shift each line so it sits left/center/right inside the block
    // width (or max_width if it was set and is larger).
    if (opts->align != TEXT_ALIGN_LEFT) {
        float box = (opts->max_width > out->width) ? opts->max_width : out->width;

        // measure per-line right edge so we know how much to shift each one.
        // small allocation on the stack-ish darray, cheap.
        float *line_w = NULL;
        for (int i = 0; i < out->line_count; i++) darr_push(line_w, 0.0f);
        for (int i = 0; i < out->count; i++) {
            text_placement *pl = &out->items[i];
            float right = pl->x + (float)pl->g->w;
            if (right > line_w[pl->line]) line_w[pl->line] = right;
        }
        for (int i = 0; i < out->count; i++) {
            text_placement *pl = &out->items[i];
            float slack = box - line_w[pl->line];
            if (slack < 0) slack = 0;
            pl->x += (opts->align == TEXT_ALIGN_CENTER) ? slack * 0.5f : slack;
        }
        darr_free(line_w);
    }

    return 1;
}

void text_layout_free(text_layout *l) {
    darr_free(l->items);
    memset(l, 0, sizeof *l);
}

void text_layout_measure(const text_font *font, const char *s,
                         const text_layout_opts *opts,
                         float *out_w, float *out_h) {
    text_layout l;
    if (!text_layout_run(font, s, opts, &l)) {
        if (out_w) *out_w = 0;
        if (out_h) *out_h = 0;
        return;
    }
    if (out_w) *out_w = l.width;
    if (out_h) *out_h = l.height;
    text_layout_free(&l);
}
