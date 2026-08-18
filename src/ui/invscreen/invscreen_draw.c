#include "invscreen_draw.h"

#include <stdio.h>
#include <string.h>

// panel tones. dark translucent backing, slightly lighter cells, lighter still
// border. nudged to sit ok over the game world behind it.
#define COL_PANEL_FILL   wg_rgba_make(24,  24,  28,  235)
#define COL_PANEL_BORDER wg_rgba_make(70,  70,  78,  255)
#define COL_CELL_FILL    wg_rgba_make(48,  48,  54,  255)
#define COL_CELL_BORDER  wg_rgba_make(90,  90,  98,  255)
#define COL_CELL_HOVER   wg_rgba_make(110, 110, 130, 255)
#define COL_TEXT         WG_WHITE
#define COL_TOOLTIP_BG   wg_rgba_make(16,  16,  20,  240)

// rough glyph advance for the bitmap font at scale 1. used to right-align the
// little stack count in the bottom corner of a cell.
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

    if (s->count > 1) {
        char buf[8];
        snprintf(buf, sizeof buf, "%d", s->count);
        // bottom-right, a couple px in. right-align by advancing left per glyph.
        float w  = (float)strlen(buf) * GLYPH_W;
        float tx = cell.x + cell.w - w - 2.0f;
        float ty = cell.y + cell.h - GLYPH_H - 1.0f;
        // shadow then text, like the rest of the hud.
        wg_draw_text(dl, tx + 1, ty + 1, buf, 1.0f, wg_rgba_fade(WG_SHADOW, alpha));
        wg_draw_text(dl, tx, ty, buf, 1.0f, wg_rgba_fade(COL_TEXT, alpha));
    }
}

void invscreen_draw_panel(wg_draw_list *dl, const invscreen_layout *L,
                          const invscreen_model *m, int hover, float alpha) {
    // chrome.
    wg_draw_rect(dl, L->panel, wg_rgba_fade(COL_PANEL_FILL, alpha));
    wg_draw_border(dl, L->panel, wg_rgba_fade(COL_PANEL_BORDER, alpha), 2.0f);

    wg_draw_text(dl, L->title.x, L->title.y, "inventory", 1.0f,
                 wg_rgba_fade(COL_TEXT, alpha));

    // the little arrow between craft input and output. two stacked lines making
    // a fat "->". purely decorative.
    {
        vec2 c = wg_rect_center(L->craft_arrow);
        float hl = L->craft_arrow.w * 0.35f;
        wg_rgba ac = wg_rgba_fade(COL_CELL_BORDER, alpha);
        wg_draw_line(dl, c.x - hl, c.y, c.x + hl, c.y, ac, 3.0f);
        wg_draw_line(dl, c.x + hl - 5, c.y - 5, c.x + hl, c.y, ac, 3.0f);
        wg_draw_line(dl, c.x + hl - 5, c.y + 5, c.x + hl, c.y, ac, 3.0f);
    }

    // every addressable cell, region by region.
    for (int r = 0; r < INVSCR_REGION_COUNT; r++) {
        int base = invscreen_model_region_base(r);
        int cnt  = invscreen_model_region_count(r);
        for (int i = 0; i < cnt; i++) {
            int idx = base + i;
            const invscreen_slot *s = invscreen_model_at_c(m, idx);
            draw_cell(dl, L->cell[idx], s, idx == hover, alpha);
        }
    }
}

void invscreen_draw_held(wg_draw_list *dl, const invscreen_model *m,
                         float mx, float my, float alpha) {
    const invscreen_slot *h = invscreen_model_at_c(m, m->held_index);
    if (!h || invscreen_slot_is_empty(h)) return;

    float s = INVSCR_SLOT_SIZE - INVSCR_SLOT_INSET * 2;
    wg_rect icon = wg_rect_make(mx - s * 0.5f, my - s * 0.5f, s, s);
    wg_draw_rect(dl, icon, wg_rgba_fade(invscreen_block_color(h->block), alpha));

    if (h->count > 1) {
        char buf[8];
        snprintf(buf, sizeof buf, "%d", h->count);
        float w = (float)strlen(buf) * GLYPH_W;
        float tx = icon.x + icon.w - w;
        float ty = icon.y + icon.h - GLYPH_H;
        wg_draw_text(dl, tx + 1, ty + 1, buf, 1.0f, wg_rgba_fade(WG_SHADOW, alpha));
        wg_draw_text(dl, tx, ty, buf, 1.0f, wg_rgba_fade(COL_TEXT, alpha));
    }
}

void invscreen_draw_tooltip(wg_draw_list *dl, const invscreen_tooltip *t,
                            float mx, float my, int screen_w, int screen_h) {
    if (!t->visible || t->nlines <= 0) return;

    float pad = 5.0f;
    float lh  = GLYPH_H + 3.0f;
    float w   = invscreen_tooltip_width(t, GLYPH_W) + pad * 2;
    float h   = (float)t->nlines * lh + pad * 2;

    // offset down-right from the cursor, then clamp so it never spills off.
    float x = mx + 14.0f;
    float y = my + 14.0f;
    if (x + w > screen_w) x = mx - w - 6.0f;
    if (y + h > screen_h) y = screen_h - h;
    if (x < 0) x = 0;
    if (y < 0) y = 0;

    wg_rect box = wg_rect_make(x, y, w, h);
    wg_draw_rect(dl, box, COL_TOOLTIP_BG);
    wg_draw_border(dl, box, COL_PANEL_BORDER, 1.0f);

    for (int i = 0; i < t->nlines; i++) {
        float ty = y + pad + i * lh;
        // first line full white, rest dimmed a touch like a real tooltip.
        wg_rgba c = i == 0 ? COL_TEXT : wg_rgba_make(170, 170, 175, 255);
        wg_draw_text(dl, x + pad + 1, ty + 1, t->line[i], 1.0f, WG_SHADOW);
        wg_draw_text(dl, x + pad, ty, t->line[i], 1.0f, c);
    }
}
