#ifndef RENDER_DEBUGDRAW_OVERLAY_H
#define RENDER_DEBUGDRAW_OVERLAY_H

#include "debugdraw.h"
#include "debugdraw_category.h"
#include "../text.h"

// the little corner panel: a list of categories with [x]/[ ] toggles and a
// stats line (verts/draws). this is the f3-ish debug menu. it draws straight
// to the hud text renderer in screen space — no world projection involved.
//
// keep this dead simple: it reads the mask + stats off the debugdraw and
// renders text rows. input handling (clicking/keying a row) lives in the
// caller; we just expose a hit-test helper.

typedef struct {
    text_renderer *tr;
    int   x, y;         // top-left in pixels
    int   row_h;        // pixel height per row
    int   sw, sh;       // screen size for text_draw
    int   visible;
} ddoverlay;

void debugdraw_overlay_init(ddoverlay *ov, text_renderer *tr,
                            int x, int y, int sw, int sh);

void debugdraw_overlay_toggle(ddoverlay *ov);

// draw the panel. reads category state + stats from dd.
void debugdraw_overlay_draw(ddoverlay *ov, const debugdraw *dd);

// which category row (if any) is under pixel (mx,my)? returns the category
// bit or 0 for none. lets the caller wire a mouse click to a toggle.
ddcategory debugdraw_overlay_hit(const ddoverlay *ov, int mx, int my);

#endif
