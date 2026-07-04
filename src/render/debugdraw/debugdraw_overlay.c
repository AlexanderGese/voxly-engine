#include "debugdraw_overlay.h"
#include <stdio.h>
static const ddcategory rows[] = {
    DD_CAT_GENERAL,
    DD_CAT_WORLD,
    DD_CAT_PHYSICS,
    DD_CAT_AI,
    DD_CAT_LIGHTING,
    DD_CAT_FLUID,
    DD_CAT_CAMERA,
    DD_CAT_MISC,
}
;
#define DD_OVERLAY_ROWS ((int)(sizeof rows / sizeof rows[0]))
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
char buf[64];
row_text(ov, 0, "-- debug draw --", 1.0f, 0.85f, 0.2f);
snprintf(buf, sizeof buf, "verts %d  draws %d",
             dd->stat_verts, dd->stat_draws);
row_text(ov, 1, buf, 0.7f, 0.7f, 0.7f);
for (int i = 0;
i < DD_OVERLAY_ROWS;
int first_y = ov->y + DD_OVERLAY_HEADER * ov->row_h;
if (my < first_y) return (ddcategory)0;
int row = (my - first_y) / ov->row_h;
if (row < 0 || row >= DD_OVERLAY_ROWS) return (ddcategory)0;
if (mx < ov->x || mx > ov->x + 120) return (ddcategory)0;
return rows[row];
}
