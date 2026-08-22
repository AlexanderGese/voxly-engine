#ifndef UI_WIDGETS_SLIDER_H
#define UI_WIDGETS_SLIDER_H

// value sliders. horizontal track with a draggable grab handle. dragging maps
// the cursor's x within the track straight to the value range (absolute, not
// relative) which is what people expect from a slider — grab anywhere and the
// handle jumps under the cursor.
//
// the float form is the primitive; int and the labelled helpers wrap it. all
// of them return 1 on any frame the value changed so callers can react (rebuild
// a chunk, re-tune the fog, whatever).

#include "widgets_context.h"
#include "widgets_layout.h"

// core: a slider in rect `r` editing *v, clamped to [vmin, vmax]. `fmt` is a
// printf format applied to the value for the overlay text (e.g. "%.2f"); pass
// NULL to hide the readout.
int wg_slider_rect(wg_context *ctx, wg_id id, wg_rect r,
                   float *v, float vmin, float vmax, const char *fmt);

// labelled float slider on a layout row: "label" above/left, track right.
int wg_slider_float(wg_context *ctx, wg_layout *l, const char *label,
                    float *v, float vmin, float vmax);

// int slider. snaps to whole steps; *v is rounded each frame it's dragged.
int wg_slider_int(wg_context *ctx, wg_layout *l, const char *label,
                  int *v, int vmin, int vmax);

// drag-int (no visible track fill, scrub left/right to nudge). good for values
// with no natural max, like a seed offset. `speed` = units per pixel dragged.
int wg_drag_int(wg_context *ctx, wg_layout *l, const char *label,
                int *v, float speed);

// a read-only progress bar reusing the slider chrome. loading screens, xp, etc.
void wg_progress(wg_context *ctx, wg_layout *l, float t01, const char *label);

#endif
