#include "text_layout.h"
#include "../../util/darray.h"
#include <string.h>
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
for (size_t i = 0;
i < darr_len(st->word);
i++) {
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
float wrap = st->opts.max_width;
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
for (const unsigned char *p = (const unsigned char*)s;
*p;
if (st.pen_x > st.max_line_w) st.max_line_w = st.pen_x;
out->items       = st.items;
out->count       = (int)darr_len(st.items);
out->line_count  = st.line + 1;
out->width       = st.max_line_w;
out->height      = (float)out->line_count * st.line_h;
out->line_height = st.line_h;
darr_free(st.word);
if (out_h) *out_h = l.height;
text_layout_free(&l);
}
