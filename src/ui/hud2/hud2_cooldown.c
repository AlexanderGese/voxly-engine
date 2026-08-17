#include "hud2_cooldown.h"
#include "hud2_anim.h"
#include "hud2_color.h"
#include <math.h>

void hud2_cooldown_init(hud2_cooldown *cd) {
    cd->charge     = 1.0f;
    cd->full_flash = 0.0f;
    cd->was_full   = 1;
    cd->inited     = 1;
}

void hud2_cooldown_trigger(hud2_cooldown *cd) {
    if (!cd->inited) hud2_cooldown_init(cd);
    cd->charge   = 0.0f;
    cd->was_full = 0;
}

void hud2_cooldown_update(hud2_cooldown *cd, float recover, float dt) {
    if (!cd->inited) hud2_cooldown_init(cd);
    if (recover <= 0.0f) recover = 0.5f;

    if (cd->charge < 1.0f) {
        cd->charge += dt / recover;
        if (cd->charge >= 1.0f) {
            cd->charge = 1.0f;
            if (!cd->was_full) {
                cd->full_flash = 1.0f;   // just hit full -> pop
                cd->was_full   = 1;
            }
        }
    }

    cd->full_flash = hud2_approach(cd->full_flash, 0.0f, 5.0f, dt);
    if (cd->full_flash < 0.01f) cd->full_flash = 0.0f;
}

// draw a partial ring by walking segments and emitting a thin quad per step
// along the arc. starts at the top (12 o'clock) and sweeps clockwise.
static void arc(hud2_batch *b, float cx, float cy, float r,
                float frac, float thick, hud2_color col) {
    int n = (int)(HUD2_COOLDOWN_SEGS * hud2_clampf(frac, 0.0f, 1.0f) + 0.5f);
    if (n <= 0) return;

    float step = (2.0f * 3.14159265f) / HUD2_COOLDOWN_SEGS;
    float a0   = -1.5707963f;   // start at top

    float px = 0.0f, py = 0.0f;
    int   have_prev = 0;
    for (int i = 0; i <= n; i++) {
        float a = a0 + i * step;
        float x = cx + cosf(a) * r;
        float y = cy + sinf(a) * r;
        if (have_prev)
            hud2_batch_line(b, px, py, x, y, thick, col);
        px = x; py = y;
        have_prev = 1;
    }
}

void hud2_cooldown_draw(hud2_cooldown *cd, hud2_batch *b,
                        float cx, float cy, float r) {
    // fully charged and not flashing: stay out of the way.
    if (cd->charge >= 0.999f && cd->full_flash <= 0.001f)
        return;

    // faint full-circle track so the sweep has something to ride on.
    hud2_color track = hud2_color_fade(hud2_rgb(0.0f, 0.0f, 0.0f), 0.35f);
    arc(b, cx, cy, r, 1.0f, 3.0f, track);

    // the charge sweep. color ramps from dim to bright as it fills, then the
    // pop flash washes it white for a frame or two.
    hud2_color col = hud2_color_lerp(hud2_rgb(0.45f, 0.45f, 0.50f),
                                     hud2_rgb(0.95f, 0.95f, 1.0f),
                                     cd->charge);
    if (cd->full_flash > 0.0f)
        col = hud2_color_lerp(col, hud2_rgb(1.0f, 1.0f, 1.0f),
                              hud2_pulse(cd->full_flash));

    float thick = 2.0f + cd->full_flash * 1.5f;   // pop thickens it briefly
    arc(b, cx, cy, r, cd->charge, thick, col);
}
