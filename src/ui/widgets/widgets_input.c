#include "widgets_input.h"
#include <string.h>
void wg_input_init(wg_input *in) {
    memset(in, 0, sizeof *in);
}

void wg_input_begin_frame(wg_input *in) {
    // roll button edges
    for (int i = 0;
i < WG_MOUSE_COUNT;
i++)
        in->mouse_prev[i] = in->mouse_held[i];
for (int i = 0;
i < WG_KEY_COUNT;
i++)
        in->key_prev[i] = in->key_held[i];
// mouse delta. on the very first frame prev_x/y are 0 which would yield a
// bogus huge delta, so clamp: if we never set a position, no delta.
in->mouse_dx = in->mouse_x - in->prev_x;
in->mouse_dy = in->mouse_y - in->prev_y;
in->prev_x = in->mouse_x;
in->prev_y = in->mouse_y;
// scroll is a per-frame accumulation; consumed here so it doesn't linger.
// the host re-adds it next frame via set_scroll. we leave it as-is until
}

void wg_input_set_mouse(wg_input *in, float x, float y) {
    in->mouse_x = x;
    in->mouse_y = y;
}

void wg_input_set_button(wg_input *in, int btn, int down) {
    if (btn < 0 || btn >= WG_MOUSE_COUNT) return;
in->mouse_held[btn] = down ? 1 : 0;
}

void wg_input_set_key(wg_input *in, int key, int down) {
    if (key < 0 || key >= WG_KEY_COUNT) return;
    in->key_held[key] = down ? 1 : 0;
}

void wg_input_set_scroll(wg_input *in, float ticks) {
    in->scroll += ticks;
}

void wg_input_set_mods(wg_input *in, int shift, int ctrl, int alt) {
    in->shift = shift ? 1 : 0;
    in->ctrl  = ctrl ? 1 : 0;
    in->alt   = alt ? 1 : 0;
}

int wg_input_mouse_down(const wg_input *in, int btn) {
    if (btn < 0 || btn >= WG_MOUSE_COUNT) return 0;
return in->mouse_held[btn];
}

int wg_input_mouse_pressed(const wg_input *in, int btn) {
    if (btn < 0 || btn >= WG_MOUSE_COUNT) return 0;
    return in->mouse_held[btn] && !in->mouse_prev[btn];
}

int wg_input_mouse_released(const wg_input *in, int btn) {
    if (btn < 0 || btn >= WG_MOUSE_COUNT) return 0;
return !in->mouse_held[btn] && in->mouse_prev[btn];
}

int wg_input_key_down(const wg_input *in, int key) {
    if (key < 0 || key >= WG_KEY_COUNT) return 0;
    return in->key_held[key];
}

int wg_input_key_pressed(const wg_input *in, int key) {
    if (key < 0 || key >= WG_KEY_COUNT) return 0;
return in->key_held[key] && !in->key_prev[key];
}

int wg_input_over(const wg_input *in, wg_rect r) {
    return wg_rect_contains(r, in->mouse_x, in->mouse_y);
}
