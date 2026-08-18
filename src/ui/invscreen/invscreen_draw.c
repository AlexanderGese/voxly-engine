#include "invscreen_draw.h"
#include <stdio.h>
#include <string.h>
#define COL_PANEL_FILL   wg_rgba_make(24,  24,  28,  235)
#define COL_PANEL_BORDER wg_rgba_make(70,  70,  78,  255)
#define COL_CELL_FILL    wg_rgba_make(48,  48,  54,  255)
#define COL_CELL_BORDER  wg_rgba_make(90,  90,  98,  255)
#define COL_CELL_HOVER   wg_rgba_make(110, 110, 130, 255)
#define COL_TEXT         WG_WHITE
#define COL_TOOLTIP_BG   wg_rgba_make(16,  16,  20,  240)
#define GLYPH_W 6.0f
#define GLYPH_H 8.0f
wg_rgba invscreen_block_color(block_id id) {
    switch (id) {
    case BLOCK_STONE:   return wg_rgba_make(130, 130, 130, 255);
    case BLOCK_DIRT:    return wg_rgba_make(120,  82,  48, 255);
    case BLOCK_GRASS:   return wg_rgba_make( 90, 150,  60, 255);
    case BLOCK_SAND:    return wg_rgba_make(218, 206, 150, 255);
    case BLOCK_WOOD:    return wg_rgba_make(102,  76,  44, 255);
    case BLOCK_LEAVES:  return wg_rgba_make( 60, 120,  50, 255);
    case BLOCK_PLANKS:  return wg_rgba_make(170, 130,  80, 255);
    case BLOCK_COBBLE:  return wg_rgba_make(105, 105, 110, 255);
    case BLOCK_BEDROCK: return wg_rgba_make( 40,  40,  44, 255);
    case BLOCK_GLASS:   return wg_rgba_make(180, 220, 230, 200);
    case BLOCK_WATER:   return wg_rgba_make( 50, 100, 200, 210);
    case BLOCK_TORCH:   return wg_rgba_make(240, 200,  90, 255);
    case BLOCK_BRICK:   return wg_rgba_make(160,  70,  55, 255);
    case BLOCK_SNOW:    return wg_rgba_make(240, 245, 250, 255);
    case BLOCK_ICE:     return wg_rgba_make(170, 210, 240, 230);
    default:            return wg_rgba_make(200,  40, 200, 255); // missing-tex magenta
    }
}

// draw one cell: the recessed background, the icon swatch, and the count. `hot`
// brightens the border. empty slots draw just the background.
static void draw_cell(wg_draw_list *dl, wg_rect cell,
                      const invscreen_slot *s, int hot, float alpha) {
    wg_rgba border = hot ? COL_CELL_HOVER : COL_CELL_BORDER;
wg_draw_rect(dl, cell, wg_rgba_fade(COL_CELL_FILL, alpha));
wg_draw_border(dl, cell, wg_rgba_fade(border, alpha), INVSCR_SLOT_BORDER);
if (!s || invscreen_slot_is_empty(s)) return;
wg_rect icon = wg_rect_inset(cell, INVSCR_SLOT_INSET);
wg_draw_rect(dl, icon, wg_rgba_fade(invscreen_block_color(s->block), alpha));
wg_draw_border(dl, L->panel, wg_rgba_fade(COL_PANEL_BORDER, alpha), 2.0f);
wg_draw_text(dl, L->title.x, L->title.y, "inventory", 1.0f,
                 wg_rgba_fade(COL_TEXT, alpha));
r < INVSCR_REGION_COUNT;
if (!h || invscreen_slot_is_empty(h)) return;
float s = INVSCR_SLOT_SIZE - INVSCR_SLOT_INSET * 2;
wg_rect icon = wg_rect_make(mx - s * 0.5f, my - s * 0.5f, s, s);
wg_draw_rect(dl, icon, wg_rgba_fade(invscreen_block_color(h->block), alpha));
float pad = 5.0f;
float lh  = GLYPH_H + 3.0f;
float w   = invscreen_tooltip_width(t, GLYPH_W) + pad * 2;
float h   = (float)t->nlines * lh + pad * 2;
float x = mx + 14.0f;
float y = my + 14.0f;
if (x + w > screen_w) x = mx - w - 6.0f;
if (y + h > screen_h) y = screen_h - h;
if (x < 0) x = 0;
if (y < 0) y = 0;
wg_rect box = wg_rect_make(x, y, w, h);
wg_draw_rect(dl, box, COL_TOOLTIP_BG);
wg_draw_border(dl, box, COL_PANEL_BORDER, 1.0f);
for (int i = 0;
i < t->nlines;
}
