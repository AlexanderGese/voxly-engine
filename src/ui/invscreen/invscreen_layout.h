#ifndef UI_INVSCREEN_LAYOUT_H
#define UI_INVSCREEN_LAYOUT_H

// container layout. given the window size this computes a screen rect for every
// slot plus the outer panel chrome. it's all derived from the config constants
// so resizing the window just re-runs this. nothing here mutates the model.

#include "../widgets/widgets_types.h"   // wg_rect
#include "invscreen_config.h"

typedef struct {
    wg_rect panel;        // outer background panel
    wg_rect title;        // title bar strip at the top of the panel
    wg_rect grid;         // backpack grid bounding box
    wg_rect hotbar;       // hotbar strip bounding box
    wg_rect craft_in;     // crafting input bounding box
    wg_rect craft_out;    // single output cell
    wg_rect craft_arrow;  // little gap between input grid and output

    // per-slot cell rects, parallel to the model's absolute index space.
    // entries past the addressable regions (the held slot) are left zeroed.
    wg_rect cell[INVSCR_SLOT_TOTAL];
} invscreen_layout;

// recompute everything for a screen of (sw, sh) pixels. the panel is centered.
void invscreen_layout_build(invscreen_layout *L, int sw, int sh);

// hit test: returns the absolute slot index under (px, py) or INVSCR_NO_SLOT.
// only the addressable regions are tested; the held slot has no rect.
int invscreen_layout_hit(const invscreen_layout *L, float px, float py);

// is the point anywhere inside the panel chrome at all. used to decide whether a
// click should pass through to the game or get eaten by the screen.
int invscreen_layout_in_panel(const invscreen_layout *L, float px, float py);

#endif
