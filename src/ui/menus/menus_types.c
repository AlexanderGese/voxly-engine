#include "menus_types.h"

// the defaults are deliberately middle-of-the-road. anything punchier and the
// first thing every player does is turn it back down.
void menus_settings_defaults(menus_settings *s) {
    s->master_volume   = 0.8f;
    s->music_volume    = 0.5f;
    s->mouse_sens      = 0.12f;
    s->render_distance = 8;
    s->fov             = 70;
    s->vsync           = 1;
    s->invert_y        = 0;
    s->show_fps        = 0;
    s->gamma           = 1.0f;
}

static float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static int clampi(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void menus_settings_clamp(menus_settings *s) {
    s->master_volume   = clampf(s->master_volume, 0.0f, 1.0f);
    s->music_volume    = clampf(s->music_volume,  0.0f, 1.0f);
    s->mouse_sens      = clampf(s->mouse_sens,    0.02f, 0.5f);
    s->render_distance = clampi(s->render_distance, 2, 16);
    s->fov             = clampi(s->fov, 60, 110);
    s->gamma           = clampf(s->gamma, 0.5f, 2.0f);
    // the rest are bools, normalize them so != comparisons behave
    s->vsync    = s->vsync    ? 1 : 0;
    s->invert_y = s->invert_y ? 1 : 0;
    s->show_fps = s->show_fps ? 1 : 0;
}

// floats compared with a small epsilon so a slider that rounds to "the same"
// value doesn't keep the apply button blinking forever.
static int fneq(float a, float b) {
    float d = a - b;
    if (d < 0) d = -d;
    return d > 0.0005f;
}

int menus_settings_differs(const menus_settings *a, const menus_settings *b) {
    if (fneq(a->master_volume, b->master_volume)) return 1;
    if (fneq(a->music_volume,  b->music_volume))  return 1;
    if (fneq(a->mouse_sens,    b->mouse_sens))    return 1;
    if (fneq(a->gamma,         b->gamma))         return 1;
    if (a->render_distance != b->render_distance) return 1;
    if (a->fov             != b->fov)             return 1;
    if (a->vsync           != b->vsync)           return 1;
    if (a->invert_y        != b->invert_y)        return 1;
    if (a->show_fps        != b->show_fps)        return 1;
    return 0;
}
