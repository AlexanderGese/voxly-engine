#ifndef UI_WIDGETS_LAYOUT_H
#define UI_WIDGETS_LAYOUT_H
// auto-layout for the widget layer. most ui is "stack a column of rows", so the
// default mode is a vertical cursor: each widget asks the layout for its rect,
// gets one the full content width by row_height tall, and the cursor advances.
#include "widgets_context.h"
typedef enum {
    WG_LAYOUT_COL = 0,   // vertical stack (default)
    WG_LAYOUT_ROW,       // horizontal strip
} wg_layout_dir;
typedef struct {
    wg_rect       bounds;     // the area we're laying into (content rect)
    float         cursor;     // along-axis offset consumed so far, from origin
    float         cross;      // for rows: along-axis position within the row
    wg_layout_dir dir;
    float         row_h;      // default size along the stacking axis
    float         spacing;    // gap between items
    float         max_extent; // high-water mark of consumed space (for sizing panels)

    // row sub-mode bookkeeping. when we're inside a begin_row, items consume
    // columns of this strip instead of advancing the main cursor.
    int   in_row;
    float row_y;              // top of the active row strip
    float row_col_x;          // next column's left edge
    float row_avail;          // width left in the strip
    int   row_cols_left;      // columns still to hand out (for equal split)
    float row_strip_h;        // height of the active row strip
} wg_layout;
void   wg_layout_begin(wg_layout *l, wg_context *ctx, wg_rect area);
wg_rect wg_layout_row(wg_layout *l, wg_context *ctx, float h);
wg_rect wg_layout_row_frac(wg_layout *l, wg_context *ctx, float frac, float h);
void    wg_layout_begin_row(wg_layout *l, wg_context *ctx, int cols, float h);
wg_rect wg_layout_cell(wg_layout *l, wg_context *ctx);
void    wg_layout_end_row(wg_layout *l);
void    wg_layout_gap(wg_layout *l, float px);
float   wg_layout_used_height(const wg_layout *l);
#endif
