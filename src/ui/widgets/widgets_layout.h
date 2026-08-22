#ifndef UI_WIDGETS_LAYOUT_H
#define UI_WIDGETS_LAYOUT_H

// auto-layout for the widget layer. most ui is "stack a column of rows", so the
// default mode is a vertical cursor: each widget asks the layout for its rect,
// gets one the full content width by row_height tall, and the cursor advances.
//
// for toolbars and side-by-side controls there's a row mode: begin_row carves
// the next strip into N equal (or weighted) columns and hands them out left to
// right. rows nest one level deep, which covers everything i've actually needed.
//
// layouts are plain values living on the stack in the calling scope — no allocs.

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

// start a column layout filling `area`. pulls metrics from the ctx style.
void   wg_layout_begin(wg_layout *l, wg_context *ctx, wg_rect area);

// next single-line widget rect, full content width, `row_h` tall (style default
// if h <= 0). advances the cursor.
wg_rect wg_layout_row(wg_layout *l, wg_context *ctx, float h);

// like row() but you control the width fraction (0..1 of content width). the
// rest of the line is left empty. handy for half-width fields.
wg_rect wg_layout_row_frac(wg_layout *l, wg_context *ctx, float frac, float h);

// open a horizontal strip `h` tall (style default if <=0) split into `cols`
// equal columns. each wg_layout_cell() returns the next column.
void    wg_layout_begin_row(wg_layout *l, wg_context *ctx, int cols, float h);
wg_rect wg_layout_cell(wg_layout *l, wg_context *ctx);
void    wg_layout_end_row(wg_layout *l);

// drop `px` of vertical space (or horizontal inside a row).
void    wg_layout_gap(wg_layout *l, float px);

// total content size consumed so far, including spacing. panels use this to
// auto-fit their height when they were opened with an "auto" size.
float   wg_layout_used_height(const wg_layout *l);

#endif
