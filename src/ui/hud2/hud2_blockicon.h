#ifndef UI_HUD2_BLOCKICON_H
#define UI_HUD2_BLOCKICON_H

#include "../../world/block.h"
#include "hud2_batch.h"
#include "hud2_color.h"

// flat-shaded fake-3d block icons for the hotbar. we dont have the atlas
// wired into the hud2 shader (it only knows solid colors) so we cheat: draw
// a little iso cube using three shaded faces of the block's base color.
// looks surprisingly ok at 32px. the atlas version is a someday-maybe.

hud2_color hud2_block_color(block_id id);

// draw an iso cube centered in the box (x,y,w,h). scale is 0..1, lets the
// hotbar do a little pop animation on the selected slot.
void hud2_block_icon(hud2_batch *b, block_id id,
                     float x, float y, float w, float h, float scale);

#endif
