#include "debug.h"
#include "../util/log.h"

#include <stdio.h>

static int shown = 0;
static int frame_counter = 0;
static double avg_dt = 0.016;

void debug_toggle(void) { shown = !shown; }
int  debug_visible(void) { return shown; }

void debug_frame(const player *p, const world *w, float dt) {
    avg_dt = avg_dt * 0.9 + dt * 0.1;
    frame_counter++;
    if (!shown) return;
    if (frame_counter % 30 != 0) return;

    double fps = avg_dt > 0 ? 1.0 / avg_dt : 0;
    fprintf(stderr,
        "[F3] fps=%.1f  pos=%.1f,%.1f,%.1f  yaw=%.2f pitch=%.2f  chunks=%d\n",
        fps, p->pos.x, p->pos.y, p->pos.z, p->yaw, p->pitch, w->count);
}
