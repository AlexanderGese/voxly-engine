#include "widgets_layout.h"

void wg_layout_begin(wg_layout *l, wg_context *ctx, wg_rect area) {
    l->bounds     = area;
    l->cursor     = 0.0f;
    l->cross      = 0.0f;
    l->dir        = WG_LAYOUT_COL;
    l->row_h      = ctx->style.row_height;
    l->spacing    = ctx->style.spacing;
    l->max_extent = 0.0f;
    l->in_row     = 0;
    l->row_y      = 0.0f;
    l->row_col_x  = 0.0f;
    l->row_avail  = 0.0f;
    l->row_cols_left = 0;
    l->row_strip_h = 0.0f;
}

static void bump_extent(wg_layout *l, float consumed) {
    if (consumed > l->max_extent) l->max_extent = consumed;
}

wg_rect wg_layout_row(wg_layout *l, wg_context *ctx, float h) {
    return wg_layout_row_frac(l, ctx, 1.0f, h);
}

wg_rect wg_layout_row_frac(wg_layout *l, wg_context *ctx, float frac, float h) {
    if (h <= 0.0f) h = l->row_h;
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

void wg_layout_begin_row(wg_layout *l, wg_context *ctx, int cols, float h) {
    if (h <= 0.0f) h = l->row_h;
    if (cols < 1) cols = 1;

    l->in_row        = 1;
    l->row_y         = l->bounds.y + l->cursor;
    l->row_col_x     = l->bounds.x;
    l->row_avail     = l->bounds.w;
    l->row_cols_left = cols;
    l->row_strip_h   = h;
    (void)ctx;
}

wg_rect wg_layout_cell(wg_layout *l, wg_context *ctx) {
    if (!l->in_row) {
        // misuse: cell() outside a row. fall back to a normal full row so we at
        // least return something usable instead of garbage.
        return wg_layout_row(l, ctx, 0);
    }
    int cols = l->row_cols_left > 0 ? l->row_cols_left : 1;
    // equal split of whatever width is left, minus the gaps between remaining
    // columns. recomputing per-cell keeps the last column flush to the edge.
    float gaps = l->spacing * (float)(cols - 1);
    float w = (l->row_avail - gaps) / (float)cols;
    if (w < 0.0f) w = 0.0f;

    wg_rect r = wg_rect_make(l->row_col_x, l->row_y, w, l->row_strip_h);

    l->row_col_x += w + l->spacing;
    l->row_avail -= w + l->spacing;
    if (l->row_cols_left > 0) l->row_cols_left--;
    return r;
}

void wg_layout_end_row(wg_layout *l) {
    if (!l->in_row) return;
    l->in_row = 0;
    l->cursor += l->row_strip_h + l->spacing;
    bump_extent(l, l->cursor);
}

void wg_layout_gap(wg_layout *l, float px) {
    if (l->in_row) {
        l->row_col_x += px;
        l->row_avail -= px;
    } else {
        l->cursor += px;
        bump_extent(l, l->cursor);
    }
}

float wg_layout_used_height(const wg_layout *l) {
    // trailing spacing is included in cursor; trim one gap so an auto panel
    // doesn't get a dangling margin at the bottom.
    float h = l->max_extent;
    if (h > l->spacing) h -= l->spacing;
    return h;
}
