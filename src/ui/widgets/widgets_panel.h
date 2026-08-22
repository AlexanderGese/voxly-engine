#ifndef UI_WIDGETS_PANEL_H
#define UI_WIDGETS_PANEL_H

// panels / windows. a panel is a titled, optionally draggable box that owns a
// content rect and a layout. you call begin, stuff widgets into the returned
// layout via the *_panel layout helpers (or just panel->layout), then end.
//
// because we're immediate mode the panel's position/open-state can't live
// inside the call — it lives in a small wg_panel struct the caller keeps around
// (one per window). that's the one bit of retained state in the whole module
// and it's deliberately tiny: a rect, a couple of flags.

#include "widgets_context.h"
#include "widgets_layout.h"

enum {
    WG_PANEL_TITLED    = 1 << 0,   // draw a titlebar
    WG_PANEL_MOVABLE   = 1 << 1,   // titlebar drags the window
    WG_PANEL_AUTOSIZE  = 1 << 2,   // height grows to fit content next frame
    WG_PANEL_NOCLIP    = 1 << 3,   // don't push a clip rect (rare)
};

typedef struct {
    wg_rect rect;        // outer rect, persists across frames (drag moves it)
    int     flags;
    int     open;        // collapsed? 0 = body hidden, titlebar still shown

    // filled by begin(), valid until end(): the content layout to add widgets to.
    wg_layout layout;
    wg_rect   content;   // inner content rect (inside padding + titlebar)
    int       last_content_h; // measured height, used by AUTOSIZE next frame
} wg_panel;

// one-time setup. position + initial size + flags.
void wg_panel_init(wg_panel *p, float x, float y, float w, float h, int flags);

// begin a panel. draws chrome, handles titlebar drag/collapse, pushes a clip,
// and prepares p->layout. returns 1 if the body is open (widgets should be
// emitted) or 0 if collapsed (skip to end). `title` may be NULL if untitled.
int  wg_panel_begin(wg_context *ctx, wg_panel *p, const char *title);

// end a panel: pop clip, finalize autosize, draw the resize affordance.
void wg_panel_end(wg_context *ctx, wg_panel *p);

// a free-floating tooltip box at (x,y). draws on top, no interaction. handy for
// hover hints; caller decides when to show it.
void wg_tooltip(wg_context *ctx, float x, float y, const char *text);

#endif
