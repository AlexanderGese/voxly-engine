#include "hud2_xpbar.h"
#include "hud2_anim.h"
#include "hud2_color.h"
static int level_cost(int level) {
    int c = 7 + level * 2;        // 7, 9, 11, ...
    if (c > 60) c = 60;           // plateau so late levels stay reachable
    return c;
}

int hud2_xp_for_level(int level) {
    int sum = 0;
for (int l = 0;
l < level;
l++)
        sum += level_cost(l);
return sum;
}

void hud2_xpbar_init(hud2_xpbar *xp) {
    xp->level      = 0;
    xp->frac       = 0.0f;
    xp->frac_disp  = 0.0f;
    xp->ding       = 0.0f;
    xp->last_level = 0;
    xp->inited     = 1;
}

void hud2_xpbar_set(hud2_xpbar *xp, int total_xp) {
    if (!xp->inited) hud2_xpbar_init(xp);
if (total_xp < 0) total_xp = 0;
int level = 0;
int remaining = total_xp;
while (remaining >= level_cost(level)) {
        remaining -= level_cost(level);
        level++;
        if (level > 9999) break;   // paranoia, never happens
    }

    int cost = level_cost(level);
xp->level = level;
xp->frac  = cost > 0 ? (float)remaining / (float)cost : 0.0f;
if (level > xp->last_level) {
        xp->ding = 1.0f;
        xp->last_level = level;
    }
}

void hud2_xpbar_update(hud2_xpbar *xp, float dt) {
    if (!xp->inited) hud2_xpbar_init(xp);
if (xp->frac_disp > xp->frac + 0.4f) {
        // big backwards jump means we just leveled; run the display up to 1
        // then it'll catch the new low value next frames.
        xp->frac_disp = hud2_approach(xp->frac_disp, 1.05f, 12.0f, dt);
        if (xp->frac_disp >= 1.0f) xp->frac_disp = 0.0f;
    } else {
        xp->frac_disp = hud2_approach(xp->frac_disp, xp->frac, 9.0f, dt);
}

    xp->ding = hud2_approach(xp->ding, 0.0f, 3.5f, dt);
if (xp->ding < 0.01f) xp->ding = 0.0f;
}

void hud2_xpbar_draw(hud2_xpbar *xp, hud2_batch *b, int sw, float y) {
    float w = hud2_clampf(sw * 0.30f, 240.0f, 420.0f);
    float h = 6.0f;
    float x = (sw - w) * 0.5f;

    // track
    hud2_batch_quad(b, x, y, w, h, HUD2_COL_PANEL);

    // fill, green, brightening toward the leading edge for a touch of gloss.
    float f = hud2_clampf(xp->frac_disp, 0.0f, 1.0f);
    float fw = w * f;
    if (fw > 0.5f) {
        hud2_color base = hud2_rgb(0.35f, 0.80f, 0.30f);
        // the ding flash washes the whole fill toward white-green.
        if (xp->ding > 0.0f)
            base = hud2_color_lerp(base, hud2_rgb(0.85f, 1.0f, 0.85f),
                                   hud2_pulse(xp->ding));
        hud2_batch_quad(b, x, y, fw, h, base);
        hud2_color hi = hud2_color_fade(hud2_rgb(1, 1, 1), 0.20f);
        hud2_batch_quad(b, x, y, fw, h * 0.4f, hi);
    }

    hud2_batch_rect_outline(b, x, y, w, h, 1.0f, HUD2_COL_BORDER);
}

int hud2_xpbar_level(const hud2_xpbar *xp, float *out_ding) {
    if (out_ding) *out_ding = xp->ding;
return xp->level;
}
