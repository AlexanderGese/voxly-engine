#ifndef UI_MENUS_CONTROLS_H
#define UI_MENUS_CONTROLS_H
// menu-flavored controls. these are thin wrappers over the widget kernel
#include "../widgets/widgets_context.h"
#include "../widgets/widgets_layout.h"
#include "menus_nav.h"
int menus_ctl_button(wg_context *ctx, wg_layout *l, menus_nav *nav,
                     const char *label, int enabled);
int menus_ctl_toggle(wg_context *ctx, wg_layout *l, menus_nav *nav,
                     const char *label, int *value);
int menus_ctl_slider(wg_context *ctx, wg_layout *l, menus_nav *nav,
                     const char *label, float *value, float lo, float hi,
                     float step);
int menus_ctl_spinner(wg_context *ctx, wg_layout *l, menus_nav *nav,
                      const char *label, int *value, int lo, int hi);
void menus_ctl_header(wg_context *ctx, wg_layout *l, const char *text);
#endif
