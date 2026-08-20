#ifndef UI_SETTINGS_SLIDER_H
#define UI_SETTINGS_SLIDER_H

// the slider control specialized for settings_value. unlike the generic
// widgets_slider this one understands the value's kind: floats drag continuously,
// ints/enums snap to their steps, bools render as a two-state pill. dragging is
// absolute (grab anywhere, handle jumps under the cursor) which is what the rest
// of voxl's sliders do.
//
// returns 1 on any frame the value changed so the layout can re-mark dirty.

#include "settings_value.h"
#include "../widgets/widgets_context.h"

// draw + interact with a slider for `v` inside `track`. `id` namespaces the
// widget. `readout` is the pre-formatted value text drawn at the right edge (may
// be NULL). returns 1 if the value changed.
int settings_slider_track(wg_context *ctx, wg_id id, wg_rect track,
                          settings_value *v, const char *readout);

// a toggle pill for a bool value. returns 1 on flip.
int settings_toggle_pill(wg_context *ctx, wg_id id, wg_rect r, settings_value *v);

// a cycler for an enum value: "< choice >" with prev/next hit zones plus a
// click-anywhere-advances fallback. `label` is the current choice text. returns
// the direction cycled (-1/0/+1) so the caller can apply it.
int settings_cycler(wg_context *ctx, wg_id id, wg_rect r,
                    const char *label, settings_value *v);

#endif
