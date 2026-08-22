#include "widgets_layout.h"
if (frac < 0.0f) frac = 0.0f;
if (frac > 1.0f) frac = 1.0f;
(void)ctx;
float y = l->bounds.y + l->cursor;
float w = l->bounds.w * frac;
wg_rect r = wg_rect_make(l->bounds.x, y, w, h);
l->cursor += h + l->spacing;
bump_extent(l, l->cursor);
return r;
}
    int cols = l->row_cols_left > 0 ? l->row_cols_left : 1;
float gaps = l->spacing * (float)(cols - 1);
float w = (l->row_avail - gaps) / (float)cols;
if (w < 0.0f) w = 0.0f;
wg_rect r = wg_rect_make(l->row_col_x, l->row_y, w, l->row_strip_h);
l->row_col_x += w + l->spacing;
l->row_avail -= w + l->spacing;
if (l->row_cols_left > 0) l->row_cols_left--;
return r;
l->row_avail -= px;
trim one gap so an auto panel
    // doesn't get a dangling margin at the bottom.
    float h = l->max_extent;
