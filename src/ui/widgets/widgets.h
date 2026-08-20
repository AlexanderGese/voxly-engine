#ifndef UI_WIDGETS_H
#define UI_WIDGETS_H

// immediate-mode widget toolkit. one include to pull the whole thing in.
//
// the usual loop, once per frame:
//
// wg_input_set_mouse(&ctx.input, mx, my);
// wg_input_set_button(&ctx.input, WG_MOUSE_LEFT, lmb_down);
// wg_input_set_scroll(&ctx.input, wheel);
// wg_begin(&ctx, screen_w, screen_h, dt);
//
// if (wg_panel_begin(&ctx, &settings_panel, "settings")) {
// wg_layout *l = &settings_panel.layout;
// wg_slider_int(&ctx, l, "render dist", &rd, 2, 16);
// wg_checkbox (&ctx, l, "vsync", &vsync);
// if (wg_button(&ctx, l, "apply")) apply_settings();
// }
// wg_panel_end(&ctx, &settings_panel);
//
// wg_end(&ctx);
// host_render_draw_list(&ctx.draw);   // game turns cmds into gl
//
// nothing here touches gl, glfw or the world — it records draw commands and
// reads a frozen input snapshot. the host wires both ends. keeps it portable
// across the game, the (someday) editor, and the headless layout tests.

#include "widgets_types.h"
#include "widgets_input.h"
#include "widgets_draw.h"
#include "widgets_style.h"
#include "widgets_context.h"
#include "widgets_layout.h"
#include "widgets_label.h"
#include "widgets_button.h"
#include "widgets_slider.h"
#include "widgets_checkbox.h"
#include "widgets_combo.h"
#include "widgets_panel.h"

#endif
