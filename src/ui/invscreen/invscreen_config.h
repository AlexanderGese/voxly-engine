#ifndef UI_INVSCREEN_CONFIG_H
#define UI_INVSCREEN_CONFIG_H
// tunables for the inventory/crafting screen. screen-space, pixels, top-left
// origin (y grows down) like the rest of the hud. dont read too much into the
// exact numbers, i nudged them until it looked right at 1280x720.
#include "../../config.h"
// a single slot cell. square. the icon sits inset inside the border.
#define INVSCR_SLOT_SIZE      40.0f
#define INVSCR_SLOT_GAP       4.0f      // gap between adjacent cells
#define INVSCR_SLOT_INSET     3.0f      // icon padding inside the cell border
#define INVSCR_SLOT_BORDER    1.5f      // border line thickness
// the main backpack grid. classic 9 wide. rows derived from slot count.
#define INVSCR_GRID_COLS      9
#define INVSCR_GRID_ROWS      3
#define INVSCR_GRID_SLOTS     (INVSCR_GRID_COLS * INVSCR_GRID_ROWS)
#define INVSCR_HOTBAR_SLOTS   HOTBAR_SLOTS
#define INVSCR_CRAFT_DIM      3
#define INVSCR_CRAFT_SLOTS    (INVSCR_CRAFT_DIM * INVSCR_CRAFT_DIM)
enum {
    INVSCR_REGION_GRID = 0,
    INVSCR_REGION_HOTBAR,
    INVSCR_REGION_CRAFT_IN,
    INVSCR_REGION_CRAFT_OUT,
    INVSCR_REGION_COUNT
}
;
#define INVSCR_SLOT_BASE_GRID      0
#define INVSCR_SLOT_BASE_HOTBAR    (INVSCR_SLOT_BASE_GRID   + INVSCR_GRID_SLOTS)
#define INVSCR_SLOT_BASE_CRAFT_IN  (INVSCR_SLOT_BASE_HOTBAR + INVSCR_HOTBAR_SLOTS)
#define INVSCR_SLOT_BASE_CRAFT_OUT (INVSCR_SLOT_BASE_CRAFT_IN + INVSCR_CRAFT_SLOTS)
#define INVSCR_SLOT_TOTAL          (INVSCR_SLOT_BASE_CRAFT_OUT + 1)
#define INVSCR_STACK_MAX      64
#define INVSCR_PANEL_PAD      12.0f
#define INVSCR_SECTION_GAP    14.0f
#define INVSCR_TITLE_H        18.0f
#define INVSCR_ANIM_TIME      0.16f
#define INVSCR_TOOLTIP_DELAY  0.35f
#define INVSCR_NO_SLOT        (-1)
#endif
