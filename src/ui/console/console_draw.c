#include "console_draw.h"

#include <stdio.h>
#include <string.h>

// the panel occupies the top CONSOLE_HEIGHT_FRAC of the window. rows are
// laid out top-down; the edit line is pinned to the bottom of the panel
// with a separator just above it. we draw text-only (matching the rest of
// the hud) and fake the backing dim with a row of block glyphs behind each
// line of text would be overkill, so the integrator is expected to have
// already drawn a dim quad — we just paint glyphs on top.

int console_visible_rows(int screen_h) {
    int panel_h = (int)(screen_h * CONSOLE_HEIGHT_FRAC);
    // reserve two rows at the bottom: separator + edit line.
    int rows = (panel_h - CONSOLE_PAD * 2) / CONSOLE_LINE_H - 2;
    if (rows < 1) rows = 1;
    return rows;
}

// build the visible edit-line string with the caret rendered as an
// underscore sitting at the cursor column. blink decides whether it shows.
static void compose_edit_line(const console_t *c, char *out, int cap) {
    const console_input *in = &c->in;
    int show_caret = console_input_caret_on(in);

    int w = 0;
    out[w++] = ']';
    out[w++] = ' ';

    for (int i = 0; i <= in->len && w < cap - 1; i++) {
        if (i == in->cursor && show_caret) {
            // overwrite-style caret: print the underscore in place. if a
            // char lives here we still want to see it, so prefer the glyph
            // unless we're at end-of-line.
            if (i == in->len) { out[w++] = '_'; break; }
        }
        if (i < in->len) out[w++] = in->buf[i];
    }
    out[w] = 0;
}

void uiconsole_draw(const console_t *c, text_renderer *t, int sw, int sh) {
    if (!c->open) return;

    int rows = console_visible_rows(sh);
    int x = CONSOLE_PAD;

    // scrollback, top row first. row 0 is the top of the window.
    for (int r = 0; r < rows; r++) {
        const console_line *ln = console_buffer_visible(&c->buf, r, rows);
        if (!ln) continue;   // empty slot up top before the buffer fills
        console_rgb col = console_severity_color(ln->sev);
        int y = CONSOLE_PAD + r * CONSOLE_LINE_H;
        text_draw(t, ln->text, x, y, col.r, col.g, col.b, sw, sh);
    }

    // separator row of dashes, sized to roughly span the panel width.
    int sep_y = CONSOLE_PAD + rows * CONSOLE_LINE_H;
    char sep[96];
    int dashes = (sw - CONSOLE_PAD * 2) / 8;   // ~8px per glyph
    if (dashes > (int)sizeof sep - 1) dashes = (int)sizeof sep - 1;
    if (dashes < 0) dashes = 0;
    memset(sep, '-', (size_t)dashes);
    sep[dashes] = 0;
    text_draw(t, sep, x, sep_y, 0.40f, 0.40f, 0.46f, sw, sh);

    // the edit line, one row below the separator.
    char edit[CONSOLE_LINE_LEN + 8];
    compose_edit_line(c, edit, sizeof edit);
    int edit_y = sep_y + CONSOLE_LINE_H;
    text_draw(t, edit, x, edit_y, 0.95f, 0.95f, 0.70f, sw, sh);

    // scrollbar hint: if we're scrolled up off the bottom, show how far.
    if (c->buf.scroll > 0) {
        char nub[48];
        int total = console_buffer_len(&c->buf);
        snprintf(nub, sizeof nub, "-- %d more below --", c->buf.scroll);
        // park it at the far right of the bottom scrollback row.
        int nub_x = sw - (int)strlen(nub) * 8 - CONSOLE_PAD;
        int nub_y = CONSOLE_PAD + (rows - 1) * CONSOLE_LINE_H;
        text_draw(t, nub, nub_x, nub_y, 0.55f, 0.55f, 0.62f, sw, sh);
        (void)total;
    }
}
