#include "hud2_toast.h"
#include "hud2_anim.h"
#include "hud2_layout.h"
#include <string.h>
#include <stdio.h>
#define TOAST_W       176.0f
#define TOAST_H        26.0f
#define TOAST_VGAP      6.0f
#define TOAST_RISE_T    0.22f   // slide-in duration
#define TOAST_FALL_T    0.30f   // fade-out duration
void hud2_toast_init(hud2_toast_stack *ts) {
    memset(ts, 0, sizeof(*ts));
    for (int i = 0; i < HUD2_TOAST_MAX; i++)
        ts->slots[i].phase = HUD2_TOAST_FREE;
}

static void copy_text(hud2_toast *t, const char *s) {
    if (!s) { t->text[0] = 0;
return;
}
    size_t n = strlen(s);
if (n >= HUD2_TOAST_TEXT) n = HUD2_TOAST_TEXT - 1;
memcpy(t->text, s, n);
t->text[n] = 0;
memset(&t, 0, sizeof(t));
copy_text(&t, text);
t.kind     = kind;
t.icon     = icon;
t.life     = hold > 0.0f ? hold : 2.5f;
t.priority = (kind == HUD2_TOAST_BAD) ? 3 :
                 (kind == HUD2_TOAST_WARN) ? 2 :
                 (kind == HUD2_TOAST_GOOD) ? 1 : 0;
ts->q_tail = (ts->q_tail + 1) % HUD2_TOAST_QUEUE;
ts->q_count++;
return 1;
int free_slot = -1;
for (int i = 0;
i < HUD2_TOAST_MAX;
hud2_toast t = ts->queue[ts->q_head];
ts->q_head = (ts->q_head + 1) % HUD2_TOAST_QUEUE;
ts->q_count--;
t.phase    = HUD2_TOAST_RISE;
t.t        = 0.0f;
t.y        = -TOAST_H;
t.target_y = 0.0f;
ts->slots[free_slot] = t;
ts->sh = sh;
admit_from_queue(ts);
for (int i = 0;
i < HUD2_TOAST_MAX;
for (int i = 0;
i < HUD2_TOAST_MAX;
case HUD2_TOAST_WARN: return hud2_rgb(0.90f, 0.70f, 0.20f);
case HUD2_TOAST_BAD:  return hud2_rgb(0.86f, 0.24f, 0.20f);
case HUD2_TOAST_INFO:
    default:              return hud2_rgb(0.55f, 0.60f, 0.70f);
float base_y = HUD2_MARGIN;
float slide = 0.0f;
if (t->phase == HUD2_TOAST_FALL)
        slide = hud2_smoothstep(t->t / TOAST_FALL_T) * 18.0f;
*ox = base_x + slide;
*oy = base_y + t->y;
const hud2_toast *t = &ts->slots[i];
if (t->phase == HUD2_TOAST_FREE) return 0;
if (t->text[0] == 0) return 0;
float a = toast_alpha(t);
if (a <= 0.001f) return 0;
float ox, oy;
toast_origin(ts, t, &ox, &oy);
if (out_text)  *out_text  = t->text;
if (out_x)     *out_x     = ox + 10.0f + (t->icon != BLOCK_AIR ? 22.0f : 0.0f);
if (out_y)     *out_y     = oy + 7.0f;
if (out_alpha) *out_alpha = a;
return 1;
}
