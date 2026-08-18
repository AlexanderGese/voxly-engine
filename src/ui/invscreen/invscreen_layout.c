#include "invscreen_layout.h"
#include "invscreen_model.h"
#include <string.h>
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
wg_rect inner = wg_rect_inset(L->panel, INVSCR_PANEL_PAD);
L->title = wg_rect_cut(&inner, 2, INVSCR_TITLE_H);
wg_rect_cut(&inner, 2, INVSCR_SECTION_GAP);
wg_rect craft_row = wg_rect_cut(&inner, 2, craft_h);
float in_w  = block_w(INVSCR_CRAFT_DIM);
