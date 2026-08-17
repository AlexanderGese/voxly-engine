#include "hud2_hotbar.h"
#include "hud2_anim.h"
#include "hud2_color.h"
#include "hud2_blockicon.h"
void hud2_hotbar_init(hud2_hotbar *hb) {
    hb->sel_x      = 0.0f;
    hb->sel_idx    = 0;
    hb->name_fade  = 0.0f;
    hb->last_block = BLOCK_AIR;
    hb->inited     = 1;
    for (int i = 0; i < HOTBAR_SLOTS; i++)
        hb->pop[i] = 0.0f;

    hb->slot     = 0.0f;
    hb->gap      = 0.0f;
    hb->origin_x = 0.0f;
    hb->origin_y = 0.0f;
}

// recompute slot geometry from the current screen size. centered along the
// bottom edge with a small margin. slot size scales gently with width so it
// doesnt look lost on a big monitor.
static void layout(hud2_hotbar *hb, int sw, int sh) {
    float slot = hud2_clampf(sw * 0.035f, 40.0f, 56.0f);
float gap  = slot * 0.10f;
float total = HOTBAR_SLOTS * slot + (HOTBAR_SLOTS - 1) * gap;
hb->slot     = slot;
hb->gap      = gap;
hb->origin_x = (sw - total) * 0.5f;
hb->origin_y = sh - slot - 14.0f;
}

// x of the left edge of slot i.
static float slot_x(const hud2_hotbar *hb, int i) {
    return hb->origin_x + i * (hb->slot + hb->gap);
}

void hud2_hotbar_update(hud2_hotbar *hb, const inventory *inv,
                        int sw, int sh, float dt) {
    if (!hb->inited) hud2_hotbar_init(hb);
layout(hb, sw, sh);
int sel = inv->selected;
if (sel < 0) sel = 0;
if (sel >= HOTBAR_SLOTS) sel = HOTBAR_SLOTS - 1;
block_id cur = inv->slot[sel];
if (sel != hb->sel_idx || cur != hb->last_block) {
        if (sel != hb->sel_idx)
            hb->pop[sel] = 1.0f;
        hb->name_fade  = 1.0f;
        hb->sel_idx    = sel;
        hb->last_block = cur;
    }

    // slide the selection box. on first frame snap so it doesnt fly in from 0.
    float target = slot_x(hb, sel);
if (hb->sel_x <= 0.0f)
        hb->sel_x = target;
else
        hb->sel_x = hud2_approach(hb->sel_x, target, 22.0f, dt);
for (int i = 0;
i < HOTBAR_SLOTS;
i++) {
        hb->pop[i] -= dt * 2.6f;
        if (hb->pop[i] < 0.0f) hb->pop[i] = 0.0f;
    }
    hb->name_fade -= dt * 0.55f;
if (hb->name_fade < 0.0f) hb->name_fade = 0.0f;
}

void hud2_hotbar_draw(hud2_hotbar *hb, hud2_batch *b, const inventory *inv) {
    float s = hb->slot;
    float y = hb->origin_y;

    // backing panel spanning all slots, one rounded-ish dark plate. we fake
    // the rounding by just insetting; no real corner geometry here.
    float pad = 4.0f;
    float panel_x = slot_x(hb, 0) - pad;
    float panel_w = (slot_x(hb, HOTBAR_SLOTS - 1) + s) - slot_x(hb, 0) + pad * 2;
    hud2_batch_quad(b, panel_x, y - pad, panel_w, s + pad * 2, HUD2_COL_PANEL);

    // individual slot cells + their icons.
    for (int i = 0; i < HOTBAR_SLOTS; i++) {
        float x = slot_x(hb, i);
        hud2_batch_quad(b, x, y, s, s, HUD2_COL_PANEL_HI);
        hud2_batch_rect_outline(b, x, y, s, s, 1.0f, HUD2_COL_BORDER);

        block_id id = inv->slot[i];
        if (id != BLOCK_AIR) {
            // pop scales the icon up briefly using a back-out ease so it
            // overshoots then settles. base 1.0, peak ~1.18.
            float pop = hb->pop[i];
            float k   = 1.0f;
            if (pop > 0.0f)
                k = 1.0f + 0.18f * hud2_pulse(pop);
            hud2_block_icon(b, id, x, y, s, s, k);
        }
    }

    // the sliding selection box. drawn last so it sits over the cells. a
    // bright border plus a faint inner glow tint.
    float sx = hb->sel_x;
    float grow = 2.0f;
    hud2_batch_quad(b, sx - grow, y - grow, s + grow * 2, s + grow * 2,
                    hud2_color_fade(HUD2_COL_SELECT, 0.10f));
    hud2_batch_rect_outline(b, sx - grow, y - grow,
                            s + grow * 2, s + grow * 2, 2.0f, HUD2_COL_SELECT);
}

const char *hud2_hotbar_label(const hud2_hotbar *hb, const inventory *inv,
                              float *out_alpha) {
    int sel = hb->sel_idx;
block_id id = inv->slot[sel];
float a = hud2_smoothstep(hud2_clampf(hb->name_fade, 0.0f, 1.0f));
if (out_alpha) *out_alpha = a;
if (id == BLOCK_AIR) return "";
const block_info *bi = block_get(id);
return bi ? bi->name : "?";
}
