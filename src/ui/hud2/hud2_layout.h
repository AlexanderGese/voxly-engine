#ifndef UI_HUD2_LAYOUT_H
#define UI_HUD2_LAYOUT_H

// shared screen-space layout constants + a couple of anchor helpers so the
// hotbar, bars and toasts dont each invent their own margins and drift apart.
// everything is in pixels, origin top-left, matching the batcher.

#define HUD2_MARGIN        14.0f   // outer screen margin
#define HUD2_GUTTER         6.0f   // gap between stacked elements
#define HUD2_BAR_W        170.0f
#define HUD2_BAR_H         11.0f

typedef enum {
    HUD2_ANCHOR_TL = 0,
    HUD2_ANCHOR_TC,
    HUD2_ANCHOR_TR,
    HUD2_ANCHOR_BL,
    HUD2_ANCHOR_BC,
    HUD2_ANCHOR_BR,
    HUD2_ANCHOR_CENTER,
} hud2_anchor;

typedef struct {
    float x, y;
} hud2_pt;

// resolve an anchor to a pixel point on a sw x sh screen, with an inset.
hud2_pt hud2_anchor_point(hud2_anchor a, int sw, int sh, float inset);

// given a box of size (w,h) and an anchor, return the top-left so the box sits
// flush against that anchor (inset from the edges).
hud2_pt hud2_anchor_box(hud2_anchor a, int sw, int sh,
                        float w, float h, float inset);

// where the bars row sits (top y of the upper bar). the hotbar reserves the
// very bottom; bars stack just above it. keeps the two from colliding.
float hud2_bars_base_y(int sw, int sh);

#endif
