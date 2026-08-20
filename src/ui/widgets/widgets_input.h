#ifndef UI_WIDGETS_INPUT_H
#define UI_WIDGETS_INPUT_H

// per-frame input snapshot for the widget layer. the game's input.c is the
// source of truth for the real devices; this is a thin frozen copy that the
// immediate-mode code reads so a widget always sees a consistent picture for
// the whole frame even if the os pumps events mid-traversal.
//
// edges (pressed/released) are computed here from the held bit vs last frame so
// individual widgets dont have to remember anything.

#include "widgets_types.h"

enum {
    WG_MOUSE_LEFT = 0,
    WG_MOUSE_RIGHT,
    WG_MOUSE_MIDDLE,
    WG_MOUSE_COUNT
};

// text-edit-ish keys we care about for sliders/spinners. we dont do full text
// entry here, just the handful that move focus or nudge values.
enum {
    WG_KEY_NONE = 0,
    WG_KEY_TAB,
    WG_KEY_ENTER,
    WG_KEY_ESCAPE,
    WG_KEY_LEFT,
    WG_KEY_RIGHT,
    WG_KEY_UP,
    WG_KEY_DOWN,
    WG_KEY_COUNT
};

typedef struct {
    float mouse_x, mouse_y;       // current cursor, screen px
    float mouse_dx, mouse_dy;     // delta since last frame
    float prev_x, prev_y;         // internal: last frame's position
    float scroll;                 // wheel ticks this frame, +up

    uint8_t mouse_held[WG_MOUSE_COUNT];
    uint8_t mouse_prev[WG_MOUSE_COUNT];

    uint8_t key_held[WG_KEY_COUNT];
    uint8_t key_prev[WG_KEY_COUNT];

    // modifier latches, copied wholesale each frame.
    uint8_t shift, ctrl, alt;
} wg_input;

void wg_input_init(wg_input *in);

// call once at the very top of a ui frame, after you've poked the new mouse pos
// / button bits in. this rolls the prev arrays and recomputes deltas.
void wg_input_begin_frame(wg_input *in);

// setters the host calls to feed real device state in before begin_frame.
void wg_input_set_mouse(wg_input *in, float x, float y);
void wg_input_set_button(wg_input *in, int btn, int down);
void wg_input_set_key(wg_input *in, int key, int down);
void wg_input_set_scroll(wg_input *in, float ticks);
void wg_input_set_mods(wg_input *in, int shift, int ctrl, int alt);

// queries used by widgets.
int  wg_input_mouse_down(const wg_input *in, int btn);
int  wg_input_mouse_pressed(const wg_input *in, int btn);   // edge: up->down
int  wg_input_mouse_released(const wg_input *in, int btn);  // edge: down->up
int  wg_input_key_down(const wg_input *in, int key);
int  wg_input_key_pressed(const wg_input *in, int key);

// did the cursor land inside r this frame.
int  wg_input_over(const wg_input *in, wg_rect r);

#endif
