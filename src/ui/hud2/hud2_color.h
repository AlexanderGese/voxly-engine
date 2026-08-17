#ifndef UI_HUD2_COLOR_H
#define UI_HUD2_COLOR_H

// rgba color for the hud2 overlay. plain struct, kept separate from vec4
// on purpose so we can hang hud-only helpers off it without polluting math/.

typedef struct {
    float r, g, b, a;
} hud2_color;

static inline hud2_color hud2_rgba(float r, float g, float b, float a) {
    hud2_color c = { r, g, b, a };
    return c;
}

static inline hud2_color hud2_rgb(float r, float g, float b) {
    return hud2_rgba(r, g, b, 1.0f);
}

// premultiply-free lerp. t clamped by caller, we dont babysit it here.
static inline hud2_color hud2_color_lerp(hud2_color a, hud2_color b, float t) {
    hud2_color c;
    c.r = a.r + (b.r - a.r) * t;
    c.g = a.g + (b.g - a.g) * t;
    c.b = a.b + (b.b - a.b) * t;
    c.a = a.a + (b.a - a.a) * t;
    return c;
}

// scale only the alpha, leave rgb alone. used a lot for fades.
static inline hud2_color hud2_color_fade(hud2_color c, float a) {
    c.a *= a;
    return c;
}

// dim rgb toward black, keep alpha. handy for "pressed" / inactive looks.
static inline hud2_color hud2_color_dim(hud2_color c, float k) {
    c.r *= k;
    c.g *= k;
    c.b *= k;
    return c;
}

// palette. these are the colors the whole hud agrees on so it doesnt look
// like five different people picked them. (it was one person. me.)
#define HUD2_COL_PANEL      hud2_rgba(0.07f, 0.07f, 0.09f, 0.62f)
#define HUD2_COL_PANEL_HI   hud2_rgba(0.14f, 0.14f, 0.17f, 0.70f)
#define HUD2_COL_BORDER     hud2_rgba(0.55f, 0.55f, 0.60f, 0.85f)
#define HUD2_COL_SELECT     hud2_rgba(1.00f, 0.92f, 0.35f, 1.00f)
#define HUD2_COL_TEXT       hud2_rgb (0.92f, 0.92f, 0.92f)
#define HUD2_COL_SHADOW     hud2_rgba(0.00f, 0.00f, 0.00f, 0.55f)
#define HUD2_COL_HEALTH     hud2_rgb (0.86f, 0.16f, 0.16f)
#define HUD2_COL_HEALTH_LO  hud2_rgb (0.95f, 0.45f, 0.10f)
#define HUD2_COL_HUNGER     hud2_rgb (0.74f, 0.54f, 0.16f)
#define HUD2_COL_STAMINA    hud2_rgb (0.22f, 0.74f, 0.32f)
#define HUD2_COL_STAMINA_LO hud2_rgb (0.80f, 0.74f, 0.20f)
#define HUD2_COL_HURT       hud2_rgba(0.80f, 0.00f, 0.00f, 1.00f)

#endif
