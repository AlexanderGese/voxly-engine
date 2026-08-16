#include "settings_ui.h"

#include <stdio.h>

#define NUM_SETTINGS 8

static const char *labels[NUM_SETTINGS] = {
    "Render Distance",
    "FOV",
    "Mouse Sensitivity",
    "VSync",
    "Show FPS",
    "View Bobbing",
    "Smooth Lighting",
    "Max FPS",
};

void settings_ui_init(settings_ui *su) {
    su->visible = 0;
    su->cursor = 0;
    su->num_entries = NUM_SETTINGS;
}

void settings_ui_toggle(settings_ui *su) { su->visible = !su->visible; }

void settings_ui_up(settings_ui *su) {
    if (!su->visible) return;
    su->cursor = (su->cursor + su->num_entries - 1) % su->num_entries;
}

void settings_ui_down(settings_ui *su) {
    if (!su->visible) return;
    su->cursor = (su->cursor + 1) % su->num_entries;
}

void settings_ui_change(settings_ui *su, game_settings *gs, int delta) {
    if (!su->visible) return;
    switch (su->cursor) {
    case 0: gs->render_distance += delta; if (gs->render_distance < 2) gs->render_distance = 2; if (gs->render_distance > 16) gs->render_distance = 16; break;
    case 1: gs->fov += (float)delta * 5; if (gs->fov < 30) gs->fov = 30; if (gs->fov > 120) gs->fov = 120; break;
    case 2: gs->mouse_sensitivity += (float)delta * 0.0005f; if (gs->mouse_sensitivity < 0.0005f) gs->mouse_sensitivity = 0.0005f; break;
    case 3: gs->vsync = !gs->vsync; break;
    case 4: gs->show_fps = !gs->show_fps; break;
    case 5: gs->view_bobbing = !gs->view_bobbing; break;
    case 6: gs->smooth_lighting = !gs->smooth_lighting; break;
    case 7: gs->max_fps += delta * 10; if (gs->max_fps < 10) gs->max_fps = 10; if (gs->max_fps > 240) gs->max_fps = 240; break;
    }
}

static void upload_and_draw_quad(glid prog, float x, float y, float w, float h,
                                  float r, float g, float b) {
    // simplified — reuses the hud shader approach
    // in a real impl we'd batch these but for a settings menu who cares
    (void)prog; (void)x; (void)y; (void)w; (void)h; (void)r; (void)g; (void)b;
}

void settings_ui_draw(const settings_ui *su, const game_settings *gs,
                      glid prog, int sw, int sh) {
    if (!su->visible) return;
    (void)prog; (void)sw; (void)sh; (void)gs;
    // TODO: draw setting labels + values using text_renderer
    // for now this is a stub — the menu logic works but the drawing
    // needs the text system wired in
}
