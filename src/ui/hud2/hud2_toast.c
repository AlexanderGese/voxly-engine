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
    if (!s) { t->text[0] = 0; return; }
    size_t n = strlen(s);
    if (n >= HUD2_TOAST_TEXT) n = HUD2_TOAST_TEXT - 1;
    memcpy(t->text, s, n);
    t->text[n] = 0;
}

// drop the weakest queued toast to make room. returns its index or -1.
static int evict_weakest(hud2_toast_stack *ts) {
    if (ts->q_count == 0) return -1;
    int worst = -1, worst_pri = 1 << 30;
    for (int k = 0; k < ts->q_count; k++) {
        int idx = (ts->q_head + k) % HUD2_TOAST_QUEUE;
        if (ts->queue[idx].priority < worst_pri) {
            worst_pri = ts->queue[idx].priority;
            worst = idx;
        }
    }
    return worst;
}

int hud2_toast_push(hud2_toast_stack *ts, const char *text,
                    hud2_toast_kind kind, block_id icon, float hold) {
    hud2_toast t;
    memset(&t, 0, sizeof(t));
    copy_text(&t, text);
    t.kind     = kind;
    t.icon     = icon;
    t.life     = hold > 0.0f ? hold : 2.5f;
    t.priority = (kind == HUD2_TOAST_BAD) ? 3 :
                 (kind == HUD2_TOAST_WARN) ? 2 :
                 (kind == HUD2_TOAST_GOOD) ? 1 : 0;

    if (ts->q_count >= HUD2_TOAST_QUEUE) {
        // backlog full. only keep this one if it outranks the weakest pending.
        int w = evict_weakest(ts);
        if (w < 0 || ts->queue[w].priority >= t.priority)
            return 0;
        ts->queue[w] = t;
        return 1;
    }

    ts->queue[ts->q_tail] = t;
    ts->q_tail = (ts->q_tail + 1) % HUD2_TOAST_QUEUE;
    ts->q_count++;
    return 1;
}

void hud2_toast_pickup(hud2_toast_stack *ts, block_id id, int amount) {
    const block_info *bi = block_get(id);
    char buf[HUD2_TOAST_TEXT];
    snprintf(buf, sizeof(buf), "+%d %s", amount, bi ? bi->name : "?");
    hud2_toast_push(ts, buf, HUD2_TOAST_GOOD, id, 2.2f);
}

// pull one queued toast into a free visible slot, if any.
static void admit_from_queue(hud2_toast_stack *ts) {
    if (ts->q_count == 0) return;
    int free_slot = -1;
    for (int i = 0; i < HUD2_TOAST_MAX; i++) {
        if (ts->slots[i].phase == HUD2_TOAST_FREE) { free_slot = i; break; }
    }
    if (free_slot < 0) return;

    hud2_toast t = ts->queue[ts->q_head];
    ts->q_head = (ts->q_head + 1) % HUD2_TOAST_QUEUE;
    ts->q_count--;

    t.phase    = HUD2_TOAST_RISE;
    t.t        = 0.0f;
    t.y        = -TOAST_H;          // start tucked above, slides down into place
    t.target_y = 0.0f;
    ts->slots[free_slot] = t;
}

// recompute target y for each live toast by packing them top-down in slot
// order. freed gaps close up as the survivors slide.
static void repack(hud2_toast_stack *ts) {
    float y = 0.0f;
    for (int i = 0; i < HUD2_TOAST_MAX; i++) {
        if (ts->slots[i].phase == HUD2_TOAST_FREE) continue;
        ts->slots[i].target_y = y;
        y += TOAST_H + TOAST_VGAP;
    }
}

void hud2_toast_update(hud2_toast_stack *ts, int sw, int sh, float dt) {
    ts->sw = sw;
    ts->sh = sh;

    admit_from_queue(ts);

    for (int i = 0; i < HUD2_TOAST_MAX; i++) {
        hud2_toast *t = &ts->slots[i];
        if (t->phase == HUD2_TOAST_FREE) continue;

        t->t += dt;
        switch (t->phase) {
        case HUD2_TOAST_RISE:
            if (t->t >= TOAST_RISE_T) { t->phase = HUD2_TOAST_HOLD; t->t = 0.0f; }
            break;
        case HUD2_TOAST_HOLD:
            if (t->t >= t->life) { t->phase = HUD2_TOAST_FALL; t->t = 0.0f; }
            break;
        case HUD2_TOAST_FALL:
            if (t->t >= TOAST_FALL_T) { t->phase = HUD2_TOAST_FREE; }
            break;
        default: break;
        }
    }

    repack(ts);

    // smooth each toast toward its packed slot so reshuffles glide.
    for (int i = 0; i < HUD2_TOAST_MAX; i++) {
        hud2_toast *t = &ts->slots[i];
        if (t->phase == HUD2_TOAST_FREE) continue;
        t->y = hud2_approach(t->y, t->target_y, 16.0f, dt);
    }
}

// accent color per kind.
static hud2_color accent_of(hud2_toast_kind k) {
    switch (k) {
    case HUD2_TOAST_GOOD: return hud2_rgb(0.32f, 0.78f, 0.36f);
    case HUD2_TOAST_WARN: return hud2_rgb(0.90f, 0.70f, 0.20f);
    case HUD2_TOAST_BAD:  return hud2_rgb(0.86f, 0.24f, 0.20f);
    case HUD2_TOAST_INFO:
    default:              return hud2_rgb(0.55f, 0.60f, 0.70f);
    }
}

// 0..1 visibility envelope for a toast across its phases.
static float toast_alpha(const hud2_toast *t) {
    if (t->phase == HUD2_TOAST_RISE)
        return hud2_smoothstep(t->t / TOAST_RISE_T);
    if (t->phase == HUD2_TOAST_FALL)
        return 1.0f - hud2_smoothstep(t->t / TOAST_FALL_T);
    return 1.0f;
}

// top-left of a toast box on screen, factoring its animated y offset.
static void toast_origin(const hud2_toast_stack *ts, const hud2_toast *t,
                         float *ox, float *oy) {
    float base_x = ts->sw - TOAST_W - HUD2_MARGIN;
    float base_y = HUD2_MARGIN;
    // on fall, also drift right a touch so it feels like it slips off.
    float slide = 0.0f;
    if (t->phase == HUD2_TOAST_FALL)
        slide = hud2_smoothstep(t->t / TOAST_FALL_T) * 18.0f;
    *ox = base_x + slide;
    *oy = base_y + t->y;
}

void hud2_toast_draw(hud2_toast_stack *ts, hud2_batch *b) {
    for (int i = 0; i < HUD2_TOAST_MAX; i++) {
        const hud2_toast *t = &ts->slots[i];
        if (t->phase == HUD2_TOAST_FREE) continue;

        float a = toast_alpha(t);
        if (a <= 0.001f) continue;

        float ox, oy;
        toast_origin(ts, t, &ox, &oy);

        // panel + a colored accent rib down the left edge.
        hud2_batch_quad(b, ox, oy, TOAST_W, TOAST_H,
                        hud2_color_fade(HUD2_COL_PANEL, a));
        hud2_color acc = hud2_color_fade(accent_of(t->kind), a);
        hud2_batch_quad(b, ox, oy, 3.0f, TOAST_H, acc);
        hud2_batch_rect_outline(b, ox, oy, TOAST_W, TOAST_H, 1.0f,
                                hud2_color_fade(HUD2_COL_BORDER, a));

        // a progress sliver along the bottom that drains during hold, so you
        // can see it's about to expire. only meaningful while holding.
        if (t->phase == HUD2_TOAST_HOLD && t->life > 0.0f) {
            float frac = 1.0f - hud2_clampf(t->t / t->life, 0.0f, 1.0f);
            hud2_batch_quad(b, ox, oy + TOAST_H - 2.0f, TOAST_W * frac, 2.0f,
                            hud2_color_fade(acc, 0.8f));
        }
    }
}

int hud2_toast_text_at(const hud2_toast_stack *ts, int i,
                       const char **out_text, float *out_x, float *out_y,
                       float *out_alpha) {
    if (i < 0 || i >= HUD2_TOAST_MAX) return 0;
    const hud2_toast *t = &ts->slots[i];
    if (t->phase == HUD2_TOAST_FREE) return 0;
    if (t->text[0] == 0) return 0;

    float a = toast_alpha(t);
    if (a <= 0.001f) return 0;

    float ox, oy;
    toast_origin(ts, t, &ox, &oy);

    if (out_text)  *out_text  = t->text;
    // leave room for the accent rib + a small icon gutter on the left.
    if (out_x)     *out_x     = ox + 10.0f + (t->icon != BLOCK_AIR ? 22.0f : 0.0f);
    if (out_y)     *out_y     = oy + 7.0f;
    if (out_alpha) *out_alpha = a;
    return 1;
}
