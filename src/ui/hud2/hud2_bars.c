#include "hud2_bars.h"
#include "hud2_anim.h"
#include "hud2_color.h"
void hud2_bars_init(hud2_bars *bars) {
    bars->hp_disp     = 1.0f;
    bars->hp_ghost    = 1.0f;
    bars->hunger_disp = 1.0f;
    bars->stam_disp   = 1.0f;
    bars->pulse_t     = 0.0f;
    bars->inited      = 1;
}

void hud2_bars_update(hud2_bars *bars, const survival *s, float dt) {
    if (!bars->inited) hud2_bars_init(bars);
float hp     = hud2_clampf((float)s->health / (float)MAX_HEALTH, 0, 1);
float hunger = hud2_clampf((float)s->hunger / (float)MAX_HUNGER, 0, 1);
float stam   = hud2_clampf(s->stamina / (float)MAX_STAMINA, 0, 1);
bars->hp_disp     = hud2_approach(bars->hp_disp, hp, 10.0f, dt);
bars->hunger_disp = hud2_approach(bars->hunger_disp, hunger, 8.0f, dt);
bars->stam_disp   = hud2_approach(bars->stam_disp, stam, 14.0f, dt);
if (bars->hp_ghost > bars->hp_disp)
        bars->hp_ghost = hud2_approach(bars->hp_ghost, bars->hp_disp, 3.0f, dt);
else
        bars->hp_ghost = bars->hp_disp;
bars->pulse_t += dt;
}

static void draw_bar(hud2_batch *b, float x, float y, float w, float h,
                     float fill, float ghost, hud2_color col,
                     hud2_color ghost_col) {
    // panel
    hud2_batch_quad(b, x, y, w, h, HUD2_COL_PANEL);

    // ghost (recent loss) drawn under the real fill so it peeks out past it.
    if (ghost > fill + 0.001f) {
        float gw = w * ghost;
        hud2_batch_quad(b, x, y, gw, h, ghost_col);
    }

    float fw = w * fill;
    if (fw > 0.5f) {
        hud2_batch_quad(b, x, y, fw, h, col);
        // subtle highlight strip along the top of the fill for a bit of gloss
        hud2_color hi = hud2_color_fade(hud2_rgb(1, 1, 1), 0.18f);
        hud2_batch_quad(b, x, y, fw, h * 0.35f, hi);
    }

    hud2_batch_rect_outline(b, x, y, w, h, 1.5f, HUD2_COL_BORDER);
}

void hud2_bars_draw(hud2_bars *bars, hud2_batch *b,
                    int sw, int sh, float base_y) {
    (void)sh;
float cx     = sw * 0.5f;
float bar_w  = 170.0f;
float bar_h  = 11.0f;
float gap    = 12.0f;
float vgap   = 4.0f;
float lo = 1.0f - bars->hp_disp;
hud2_color hp_col = HUD2_COL_HEALTH;
float lx = cx - bar_w - gap * 0.5f;
draw_bar(b, lx, base_y, bar_w, bar_h,
             bars->hp_disp, bars->hp_ghost, hp_col, ghost_col);
hud2_color stam_col = bars->stam_disp < 0.25f
                        ? HUD2_COL_STAMINA_LO : HUD2_COL_STAMINA;
draw_bar(b, lx, base_y + bar_h + vgap, bar_w * 0.6f, bar_h * 0.6f,
             bars->stam_disp, bars->stam_disp, stam_col, ghost_col);
float rx = cx + gap * 0.5f;
draw_bar(b, rx, base_y, bar_w, bar_h,
             bars->hunger_disp, bars->hunger_disp, HUD2_COL_HUNGER, ghost_col);
}
