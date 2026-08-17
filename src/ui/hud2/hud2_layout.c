#include "hud2_layout.h"
#include "hud2_anim.h"
#include "../../config.h"

hud2_pt hud2_anchor_point(hud2_anchor a, int sw, int sh, float inset) {
    float L = inset, R = sw - inset, T = inset, B = sh - inset;
    float cx = sw * 0.5f, cy = sh * 0.5f;
    hud2_pt p;
    switch (a) {
    case HUD2_ANCHOR_TL:     p.x = L;  p.y = T;  break;
    case HUD2_ANCHOR_TC:     p.x = cx; p.y = T;  break;
    case HUD2_ANCHOR_TR:     p.x = R;  p.y = T;  break;
    case HUD2_ANCHOR_BL:     p.x = L;  p.y = B;  break;
    case HUD2_ANCHOR_BC:     p.x = cx; p.y = B;  break;
    case HUD2_ANCHOR_BR:     p.x = R;  p.y = B;  break;
    case HUD2_ANCHOR_CENTER:
    default:                 p.x = cx; p.y = cy; break;
    }
    return p;
}

hud2_pt hud2_anchor_box(hud2_anchor a, int sw, int sh,
                        float w, float h, float inset) {
    hud2_pt anchor = hud2_anchor_point(a, sw, sh, inset);
    hud2_pt tl = anchor;

    // shift the anchor point to the box top-left depending on which edge/corner
    // it represents. top row keeps y, bottom row pulls up by h, etc.
    switch (a) {
    case HUD2_ANCHOR_TL:                                  break;
    case HUD2_ANCHOR_TC:     tl.x -= w * 0.5f;            break;
    case HUD2_ANCHOR_TR:     tl.x -= w;                   break;
    case HUD2_ANCHOR_BL:     tl.y -= h;                   break;
    case HUD2_ANCHOR_BC:     tl.x -= w * 0.5f; tl.y -= h; break;
    case HUD2_ANCHOR_BR:     tl.x -= w;        tl.y -= h; break;
    case HUD2_ANCHOR_CENTER:
    default:                 tl.x -= w * 0.5f; tl.y -= h * 0.5f; break;
    }
    return tl;
}

float hud2_bars_base_y(int sw, int sh) {
    // mirror the hotbar's own slot sizing so the bars clear it. the hotbar
    // claims a slot-height strip plus its margin at the bottom; we park the
    // bars a gutter above that strip.
    float slot = hud2_clampf(sw * 0.035f, 40.0f, 56.0f);
    float hotbar_top = sh - slot - HUD2_MARGIN;
    // two stacked bars (main + the shorter secondary) plus gutters.
    float bars_block = HUD2_BAR_H + HUD2_GUTTER + HUD2_BAR_H * 0.6f;
    return hotbar_top - HUD2_GUTTER - bars_block;
}
