#ifndef UI_WIDGETS_DRAW_H
#define UI_WIDGETS_DRAW_H

// the widget layer doesn't talk to gl directly. it records a flat list of draw
// commands (filled rects, rect outlines, glyph runs) into this buffer during
// traversal, and the host drains it once at the end of the frame and turns each
// command into whatever it actually uses — the text_batch for glyphs, a simple
// colored-quad shader for rects. keeping it backend-agnostic means the same ui
// code runs in the game, the editor, and the headless layout tests.
//
// commands carry a clip rect index so panels can scissor their children without
// the immediate-mode code needing a gl scissor stack.

#include "widgets_types.h"

typedef enum {
    WG_CMD_RECT = 0,     // filled rect
    WG_CMD_BORDER,       // 1px (thickness) outline
    WG_CMD_TEXT,         // a string run, anchored top-left at (x,y)
    WG_CMD_LINE,         // a thin filled line, used for slider tracks
} wg_cmd_kind;

// text payload is stored inline up to this many bytes; longer strings are
// truncated. widget labels are short, this is plenty and avoids per-cmd allocs.
#define WG_TEXT_MAX 96

typedef struct {
    wg_cmd_kind kind;
    wg_rect     rect;        // geometry for rect/border/line (line uses x,y..w,h as a span)
    wg_rgba     color;
    float       thickness;   // border/line width in px
    int         clip;        // index into the clip stack, -1 = no clip
    float       text_scale;  // for WG_CMD_TEXT
    char        text[WG_TEXT_MAX];
} wg_cmd;

typedef struct {
    wg_cmd  *cmds;       // darray
    wg_rect *clips;      // darray, clip rect stack snapshot per push
    int      cur_clip;   // top of clip stack, -1 when empty
} wg_draw_list;

void wg_draw_init(wg_draw_list *dl);
void wg_draw_free(wg_draw_list *dl);
void wg_draw_reset(wg_draw_list *dl);

// clip stack. pushing intersects with the current top so nested panels behave.
void wg_draw_push_clip(wg_draw_list *dl, wg_rect r);
void wg_draw_pop_clip(wg_draw_list *dl);

void wg_draw_rect(wg_draw_list *dl, wg_rect r, wg_rgba c);
void wg_draw_border(wg_draw_list *dl, wg_rect r, wg_rgba c, float thickness);
void wg_draw_line(wg_draw_list *dl, float x0, float y0, float x1, float y1,
                  wg_rgba c, float thickness);
void wg_draw_text(wg_draw_list *dl, float x, float y, const char *s,
                  float scale, wg_rgba c);

// rounded-ish convenience: fill + border in the panel's two tones.
void wg_draw_panel(wg_draw_list *dl, wg_rect r, wg_rgba fill, wg_rgba border);

int  wg_draw_count(const wg_draw_list *dl);

#endif
