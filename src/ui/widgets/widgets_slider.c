#include "widgets_slider.h"
#include "widgets_label.h"
#include <stdio.h>
#include <math.h>
if (vmax <= vmin) vmax = vmin + 1e-4f;
int hovered = 0, held = 0;
wg_behavior(ctx, id, r, &hovered, &held);
float old = *v;
float grab = ctx->style.slider_grab;
float x0 = r.x + grab;
float x1 = r.x + r.w - grab;
float span = x1 - x0;
if (span < 1.0f) span = 1.0f;
}
    *v = clampf(*v, vmin, vmax);
