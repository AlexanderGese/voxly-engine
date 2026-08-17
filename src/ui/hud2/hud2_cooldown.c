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
if (n <= 0) return;
float step = (2.0f * 3.14159265f) / HUD2_COOLDOWN_SEGS;
float a0   = -1.5707963f;
float px = 0.0f, py = 0.0f;
int   have_prev = 0;
for (int i = 0;
i <= n;
hud2_color track = hud2_color_fade(hud2_rgb(0.0f, 0.0f, 0.0f), 0.35f);
arc(b, cx, cy, r, 1.0f, 3.0f, track);
hud2_color col = hud2_color_lerp(hud2_rgb(0.45f, 0.45f, 0.50f),
                                     hud2_rgb(0.95f, 0.95f, 1.0f),
                                     cd->charge);
if (cd->full_flash > 0.0f)
        col = hud2_color_lerp(col, hud2_rgb(1.0f, 1.0f, 1.0f),
                              hud2_pulse(cd->full_flash));
float thick = 2.0f + cd->full_flash * 1.5f;
arc(b, cx, cy, r, cd->charge, thick, col);
}
