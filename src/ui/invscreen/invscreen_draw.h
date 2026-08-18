#ifndef UI_INVSCREEN_DRAW_H
#define UI_INVSCREEN_DRAW_H

// turns the model + layout into widget draw commands. it doesn't touch gl; it
// records into the shared wg_draw_list and the host drains it like any other ui.
// block icons are drawn as flat colored squares tinted per block id — the real
// renderer would blit the atlas tile, but the colored-square stand-in keeps this
// file free of texture/atlas dependencies and reads fine in the layout tests.

#include "../widgets/widgets_draw.h"
#include "invscreen_layout.h"
#include "invscreen_model.h"
#include "invscreen_tooltip.h"

// a tiny palette so each block id gets a recognizable swatch. screen-space, the
// same packed rgba the widget layer uses.
wg_rgba invscreen_block_color(block_id id);

// draw the whole panel: chrome, every region's cells, their icons and counts.
// `hover` is the slot the cursor is over (for the highlight), INVSCR_NO_SLOT for
// none. `alpha` fades the entire thing for the open/close animation.
void invscreen_draw_panel(wg_draw_list *dl, const invscreen_layout *L,
                          const invscreen_model *m, int hover, float alpha);

// draw the cursor-held stack floating at (mx, my), centered on the cursor. no-op
// when nothing is held.
void invscreen_draw_held(wg_draw_list *dl, const invscreen_model *m,
                         float mx, float my, float alpha);

// draw the tooltip box near (mx, my) if it's visible. clamps to stay on screen.
void invscreen_draw_tooltip(wg_draw_list *dl, const invscreen_tooltip *t,
                            float mx, float my, int screen_w, int screen_h);

#endif
