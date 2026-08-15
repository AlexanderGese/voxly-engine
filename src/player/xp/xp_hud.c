#include "xp_hud.h"

#include <stddef.h>

#include "xp_config.h"
#include "xp_event.h"

// bar sits centered, a little above the bottom edge, minecraft-ish.
static void layout_bar(const xp_system *x, vec2 vp, xp_hud_layout *out) {
    float bw = vp.x * 0.36f;
    float bh = 7.0f;
    out->bar_size = vec2_new(bw, bh);
    out->bar_pos  = vec2_new((vp.x - bw) * 0.5f, vp.y - 56.0f);
    out->bar_fill = xp_progress(x);
    if (out->bar_fill < 0.0f) out->bar_fill = 0.0f;
    if (out->bar_fill > 1.0f) out->bar_fill = 1.0f;
    out->level = xp_level(x);
}

// map an event to a popup. returns 1 if it produced one, 0 to skip.
static int popup_from_event(const xp_event *e, vec2 vp, float life,
                            xp_hud_popup *out) {
    float t = e->age / (life > 0.0f ? life : 1.0f);
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) return 0; // expired, skip

    // popups rise as they age and fade in the last third.
    float rise = t * 40.0f;
    float alpha = t < 0.7f ? 1.0f : (1.0f - (t - 0.7f) / 0.3f);
    // scale pops to 1.2 in the first 80ms-ish then eases to 1.0.
    float scale = t < 0.08f ? 0.6f + t / 0.08f * 0.6f : 1.0f;

    switch (e->kind) {
    case XP_EV_GAIN:
        out->kind = XP_POPUP_GAIN;
        out->value = e->amount;
        // gains drift up from just above the bar, jittered horizontally by the
        // event's world x so a burst spreads out instead of stacking.
        {
            float jx = e->where.x;
            // wrap the world coord into a small pixel offset, deterministic.
            float off = (jx - (float)(int)jx) * 60.0f - 30.0f;
            out->pos = vec2_new(vp.x * 0.5f + off, vp.y - 80.0f - rise);
        }
        break;
    case XP_EV_LEVELUP:
        out->kind = XP_POPUP_LEVEL;
        out->value = e->amount; // the new level
        out->pos = vec2_new(vp.x * 0.5f - 36.0f, vp.y - 110.0f - rise);
        break;
    case XP_EV_REWARD:
        out->kind = XP_POPUP_REWARD;
        out->value = e->amount; // reward kind id
        out->pos = vec2_new(vp.x * 0.5f - 60.0f, vp.y - 130.0f - rise);
        break;
    default:
        return 0;
    }

    out->alpha = alpha;
    out->scale = scale;
    return 1;
}

void xp_hud_build(const xp_system *x, vec2 viewport, float popup_life,
                  xp_hud_layout *out) {
    layout_bar(x, viewport, out);

    out->popup_count = 0;
    // walk recent events newest-first, but write oldest-first so the renderer
    // draws fresh popups on top. we iterate the log directly.
    const xp_event_log *log = &x->log;

    // collect up to the cap, skipping expired ones.
    int n = log->count;
    if (n > XP_HUD_MAX_POPUPS) n = XP_HUD_MAX_POPUPS;

    for (int i = n - 1; i >= 0; i--) {
        const xp_event *e = xp_event_recent(log, i);
        if (!e) continue;
        if (out->popup_count >= XP_HUD_MAX_POPUPS) break;
        xp_hud_popup p;
        if (popup_from_event(e, viewport, popup_life, &p)) {
            out->popups[out->popup_count++] = p;
        }
    }
}
