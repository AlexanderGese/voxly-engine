#include "hud2_blockicon.h"
// base color per block. matches the old hud.c table so the look is consistent
// across both huds while we migrate. magenta = "i forgot to add this one".
hud2_color hud2_block_color(block_id id) {
    switch (id) {
    case BLOCK_STONE:    return hud2_rgb(0.50f, 0.50f, 0.50f);
    case BLOCK_DIRT:     return hud2_rgb(0.45f, 0.30f, 0.15f);
    case BLOCK_GRASS:    return hud2_rgb(0.30f, 0.65f, 0.20f);
    case BLOCK_SAND:     return hud2_rgb(0.85f, 0.80f, 0.55f);
    case BLOCK_WOOD:     return hud2_rgb(0.50f, 0.35f, 0.15f);
    case BLOCK_LEAVES:   return hud2_rgb(0.15f, 0.50f, 0.10f);
    case BLOCK_PLANKS:   return hud2_rgb(0.70f, 0.55f, 0.30f);
    case BLOCK_COBBLE:   return hud2_rgb(0.40f, 0.40f, 0.40f);
    case BLOCK_BEDROCK:  return hud2_rgb(0.15f, 0.15f, 0.15f);
    case BLOCK_GLASS:    return hud2_rgba(0.75f, 0.85f, 0.95f, 0.65f);
    case BLOCK_WATER:    return hud2_rgba(0.20f, 0.40f, 0.80f, 0.80f);
    case BLOCK_TORCH:    return hud2_rgb(1.00f, 0.85f, 0.20f);
    case BLOCK_BRICK:    return hud2_rgb(0.65f, 0.30f, 0.25f);
    case BLOCK_SNOW:     return hud2_rgb(0.95f, 0.95f, 0.95f);
    case BLOCK_ICE:      return hud2_rgb(0.60f, 0.80f, 0.95f);
    default:             return hud2_rgb(0.80f, 0.00f, 0.80f);
    }
}

void hud2_block_icon(hud2_batch *b, block_id id,
                     float x, float y, float w, float h, float scale) {
    if (id == BLOCK_AIR) return;
if (scale <= 0.0f) return;
hud2_color base = hud2_block_color(id);
// shrink toward center by scale. the inset gives a margin even at 1.0.
float inset = 0.16f;
float sw = w * (1.0f - inset * 2.0f) * scale;
float sh = h * (1.0f - inset * 2.0f) * scale;
float cx = x + w * 0.5f;
float cy = y + h * 0.5f;
// iso cube geometry. top is a rhombus, the two side faces are
// parallelograms. classic 2:1 dimetric-ish projection.
float hw = sw * 0.5f;
float hh = sh * 0.5f;
float th = hh * 0.5f;
// vertical extent of the top diamond half
// key points
float top_x  = cx,        top_y  = cy - hh;
// peak
float left_x = cx - hw,   left_y = cy - hh + th;
// upper-left
float right_x= cx + hw,   right_y= cy - hh + th;
// upper-right
float mid_x  = cx,        mid_y  = cy - hh + th * 2;
// front-mid (top center)
float bl_x   = cx - hw,   bl_y   = cy + hh - th;
float br_x   = cx + hw,   br_y   = cy + hh - th;
float bot_x  = cx,        bot_y  = cy + hh;
hud2_color top  = hud2_color_dim(base, 1.0f);
hud2_batch_tri(b, top_x, top_y, left_x, left_y, mid_x, mid_y, top);
hud2_batch_tri(b, top_x, top_y, mid_x, mid_y, right_x, right_y, top);
hud2_color lf = hud2_color_dim(base, 0.72f);
hud2_batch_tri(b, left_x, left_y, bl_x, bl_y, bot_x, bot_y, lf);
hud2_batch_tri(b, left_x, left_y, bot_x, bot_y, mid_x, mid_y, lf);
hud2_color rf = hud2_color_dim(base, 0.52f);
hud2_batch_tri(b, right_x, right_y, mid_x, mid_y, bot_x, bot_y, rf);
hud2_batch_tri(b, right_x, right_y, bot_x, bot_y, br_x, br_y, rf);
}
