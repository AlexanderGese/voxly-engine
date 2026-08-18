#include "invscreen_layout.h"
#include "invscreen_model.h"

#include <string.h>

// width/height of a block of `cols` x `rows` cells including the gaps between
// them. used everywhere below so the panel sizing stays consistent.
static float block_w(int cols) {
    return cols * INVSCR_SLOT_SIZE + (cols - 1) * INVSCR_SLOT_GAP;
}
static float block_h(int rows) {
    return rows * INVSCR_SLOT_SIZE + (rows - 1) * INVSCR_SLOT_GAP;
}

// place a cols x rows run of cells starting at the box's top-left, writing the
// rects into L->cell[base .. base+cols*rows). row-major, matching the model.
static void place_grid(invscreen_layout *L, wg_rect box, int base,
                       int cols, int rows) {
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            float x = box.x + c * (INVSCR_SLOT_SIZE + INVSCR_SLOT_GAP);
            float y = box.y + r * (INVSCR_SLOT_SIZE + INVSCR_SLOT_GAP);
            L->cell[base + r * cols + c] =
                wg_rect_make(x, y, INVSCR_SLOT_SIZE, INVSCR_SLOT_SIZE);
        }
    }
}

void invscreen_layout_build(invscreen_layout *L, int sw, int sh) {
    memset(L, 0, sizeof *L);

    // figure the content height first so we can center the whole thing. the
    // crafting row and the backpack grid sit side by side conceptually but we
    // stack them: craft on top, backpack below, hotbar at the very bottom.
    float craft_h  = block_h(INVSCR_CRAFT_DIM);
    float grid_h   = block_h(INVSCR_GRID_ROWS);
    float hotbar_h = block_h(1);

    float content_w = block_w(INVSCR_GRID_COLS);
    float content_h = INVSCR_TITLE_H + INVSCR_SECTION_GAP
                    + craft_h + INVSCR_SECTION_GAP
                    + grid_h  + INVSCR_SECTION_GAP
                    + hotbar_h;

    float panel_w = content_w + INVSCR_PANEL_PAD * 2;
    float panel_h = content_h + INVSCR_PANEL_PAD * 2;

    float px = (sw - panel_w) * 0.5f;
    float py = (sh - panel_h) * 0.5f;
    if (px < 0) px = 0;
    if (py < 0) py = 0;

    L->panel = wg_rect_make(px, py, panel_w, panel_h);

    // carve the panel top-down using the cut primitive from widgets_types.
    wg_rect inner = wg_rect_inset(L->panel, INVSCR_PANEL_PAD);
    L->title = wg_rect_cut(&inner, 2, INVSCR_TITLE_H);
    wg_rect_cut(&inner, 2, INVSCR_SECTION_GAP);   // gap after title

    // crafting section. the 3x3 input on the left, an arrow gap, output on the
    // right. center the input+arrow+output run horizontally in the content.
    wg_rect craft_row = wg_rect_cut(&inner, 2, craft_h);
    float in_w  = block_w(INVSCR_CRAFT_DIM);
    float out_w = INVSCR_SLOT_SIZE;
    float arrow_w = 28.0f;
    float craft_total = in_w + arrow_w + out_w;
    float craft_x = craft_row.x + (craft_row.w - craft_total) * 0.5f;

    L->craft_in   = wg_rect_make(craft_x, craft_row.y, in_w, craft_h);
    L->craft_arrow = wg_rect_make(craft_x + in_w, craft_row.y, arrow_w, craft_h);
    // output is one cell, vertically centered against the 3x3 input block.
    L->craft_out  = wg_rect_make(craft_x + in_w + arrow_w,
                                 craft_row.y + (craft_h - out_w) * 0.5f,
                                 out_w, out_w);
    wg_rect_cut(&inner, 2, INVSCR_SECTION_GAP);

    // backpack grid.
    L->grid = wg_rect_cut(&inner, 2, grid_h);
    wg_rect_cut(&inner, 2, INVSCR_SECTION_GAP);

    // hotbar strip — whatever is left, one row tall.
    L->hotbar = wg_rect_cut(&inner, 2, hotbar_h);

    // now stamp the actual per-cell rects.
    place_grid(L, L->grid, INVSCR_SLOT_BASE_GRID,
               INVSCR_GRID_COLS, INVSCR_GRID_ROWS);
    place_grid(L, L->hotbar, INVSCR_SLOT_BASE_HOTBAR,
               INVSCR_HOTBAR_SLOTS, 1);
    place_grid(L, L->craft_in, INVSCR_SLOT_BASE_CRAFT_IN,
               INVSCR_CRAFT_DIM, INVSCR_CRAFT_DIM);
    L->cell[INVSCR_SLOT_BASE_CRAFT_OUT] = L->craft_out;
}

int invscreen_layout_hit(const invscreen_layout *L, float px, float py) {
    // only the addressable regions have rects; the held slot sits at the tail
    // with a zeroed rect so it never hits.
    for (int r = 0; r < INVSCR_REGION_COUNT; r++) {
        int base = invscreen_model_region_base(r);
        int cnt  = invscreen_model_region_count(r);
        for (int i = 0; i < cnt; i++) {
            int idx = base + i;
            if (wg_rect_contains(L->cell[idx], px, py))
                return idx;
        }
    }
    return INVSCR_NO_SLOT;
}

int invscreen_layout_in_panel(const invscreen_layout *L, float px, float py) {
    return wg_rect_contains(L->panel, px, py);
}
