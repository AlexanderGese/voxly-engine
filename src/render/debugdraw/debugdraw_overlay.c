#include "debugdraw_overlay.h"

#include <stdio.h>

// categories listed in the panel, top to bottom. order is deliberate:
// the stuff you toggle most lives up top.
static const ddcategory rows[] = {
    DD_CAT_GENERAL,
    DD_CAT_WORLD,
    DD_CAT_PHYSICS,
    DD_CAT_AI,
    DD_CAT_LIGHTING,
    DD_CAT_FLUID,
    DD_CAT_CAMERA,
    DD_CAT_MISC,
};
#define DD_OVERLAY_ROWS ((int)(sizeof rows / sizeof rows[0]))

// header rows sit above the toggles (title + stats), offset the hit test
#define DD_OVERLAY_HEADER 2

void debugdraw_overlay_init(ddoverlay *ov, text_renderer *tr,
                            int x, int y, int sw, int sh) {
    ov->tr      = tr;
    ov->x       = x;
    ov->y       = y;
    ov->row_h   = 12;     // matches the 6x8 font with a little leading
    ov->sw      = sw;
    ov->sh      = sh;
    ov->visible = 0;
}

void debugdraw_overlay_toggle(ddoverlay *ov) {
    ov->visible = !ov->visible;
}

static void row_text(ddoverlay *ov, int row, const char *s,
                     float r, float g, float b) {
    int py = ov->y + row * ov->row_h;
    text_draw(ov->tr, s, ov->x, py, r, g, b, ov->sw, ov->sh);
}

void debugdraw_overlay_draw(ddoverlay *ov, const debugdraw *dd) {
    if (!ov->visible || !ov->tr) return;

    char buf[64];

    // title
    row_text(ov, 0, "-- debug draw --", 1.0f, 0.85f, 0.2f);

    // stats line. dd keeps these per-flush.
    snprintf(buf, sizeof buf, "verts %d  draws %d",
             dd->stat_verts, dd->stat_draws);
    row_text(ov, 1, buf, 0.7f, 0.7f, 0.7f);

    // one row per category with a checkbox + name
    for (int i = 0; i < DD_OVERLAY_ROWS; i++) {
        ddcategory cat = rows[i];
        int on = debugdraw_category_on(dd, cat);
        snprintf(buf, sizeof buf, "[%c] %s",
                 on ? 'x' : ' ', debugdraw_category_name(cat));

        // on rows render bright, off rows dim so the state reads at a glance
        float v = on ? 0.95f : 0.4f;
        row_text(ov, DD_OVERLAY_HEADER + i, buf, v, v, v);
    }
}

ddcategory debugdraw_overlay_hit(const ddoverlay *ov, int mx, int my) {
    if (!ov->visible) return (ddcategory)0;

    // toggle rows start after the header
    int first_y = ov->y + DD_OVERLAY_HEADER * ov->row_h;
    if (my < first_y) return (ddcategory)0;

    int row = (my - first_y) / ov->row_h;
    if (row < 0 || row >= DD_OVERLAY_ROWS) return (ddcategory)0;

    // rough x bound: rows are short, give ~120px of clickable width
    if (mx < ov->x || mx > ov->x + 120) return (ddcategory)0;

    return rows[row];
}
