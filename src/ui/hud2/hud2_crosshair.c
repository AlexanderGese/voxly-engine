#include "hud2_crosshair.h"
#include "hud2_anim.h"
#include "hud2_color.h"

void hud2_crosshair_init(hud2_crosshair *c) {
    c->style     = HUD2_CROSS_PLUS;
    c->gap       = 4.0f;
    c->len       = 7.0f;
    c->thick     = 2.0f;
    c->spread    = 0.0f;
    c->hit_flash = 0.0f;
    c->targeting = 0;
}

void hud2_crosshair_recoil(hud2_crosshair *c, float amount) {
    c->spread += amount;
    if (c->spread > 18.0f) c->spread = 18.0f;   // cap, dont let it fly apart
}

void hud2_crosshair_hit(hud2_crosshair *c) {
    c->hit_flash = 1.0f;
}

void hud2_crosshair_update(hud2_crosshair *c, int targeting, float dt) {
    c->targeting = targeting;
    // spread springs back to rest. flash decays a touch slower so you see it.
    c->spread    = hud2_approach(c->spread, 0.0f, 12.0f, dt);
    c->hit_flash = hud2_approach(c->hit_flash, 0.0f, 9.0f, dt);
    if (c->spread < 0.05f) c->spread = 0.0f;
    if (c->hit_flash < 0.01f) c->hit_flash = 0.0f;
}

static void tick(hud2_batch *b, float cx, float cy,
                 float gap, float len, float thick, hud2_color col, int dir) {
    // dir: 0=up 1=down 2=left 3=right
    switch (dir) {
    case 0: hud2_batch_quad(b, cx - thick*0.5f, cy - gap - len, thick, len, col); break;
    case 1: hud2_batch_quad(b, cx - thick*0.5f, cy + gap,       thick, len, col); break;
    case 2: hud2_batch_quad(b, cx - gap - len,  cy - thick*0.5f, len, thick, col); break;
    case 3: hud2_batch_quad(b, cx + gap,        cy - thick*0.5f, len, thick, col); break;
    }
}

void hud2_crosshair_draw(hud2_crosshair *c, hud2_batch *b, int sw, int sh) {
    float cx = sw * 0.5f;
    float cy = sh * 0.5f;

    // base white, tint slightly green-ish when aimed at a block so you get a
    // subtle "you can interact" cue. flash overrides toward bright yellow.
    hud2_color col = hud2_rgba(0.95f, 0.95f, 0.95f, 0.90f);
    if (c->targeting)
        col = hud2_color_lerp(col, hud2_rgba(0.70f, 1.0f, 0.75f, 0.95f), 0.5f);
    if (c->hit_flash > 0.0f)
        col = hud2_color_lerp(col, hud2_rgba(1.0f, 0.95f, 0.30f, 1.0f),
                              c->hit_flash);

    float gap = c->gap + c->spread;

    if (c->style == HUD2_CROSS_DOT) {
        float s = c->thick + 1.0f;
        hud2_batch_quad(b, cx - s*0.5f, cy - s*0.5f, s, s, col);
        return;
    }

    // soft drop shadow so it reads on bright skies. one px offset.
    hud2_color sh_col = hud2_color_fade(HUD2_COL_SHADOW, 0.7f);
    tick(b, cx + 1, cy + 1, gap, c->len, c->thick, sh_col, 0);
    tick(b, cx + 1, cy + 1, gap, c->len, c->thick, sh_col, 2);
    tick(b, cx + 1, cy + 1, gap, c->len, c->thick, sh_col, 3);
    if (c->style != HUD2_CROSS_T)
        tick(b, cx + 1, cy + 1, gap, c->len, c->thick, sh_col, 1);

    tick(b, cx, cy, gap, c->len, c->thick, col, 0);
    tick(b, cx, cy, gap, c->len, c->thick, col, 2);
    tick(b, cx, cy, gap, c->len, c->thick, col, 3);
    if (c->style != HUD2_CROSS_T)
        tick(b, cx, cy, gap, c->len, c->thick, col, 1);
}
