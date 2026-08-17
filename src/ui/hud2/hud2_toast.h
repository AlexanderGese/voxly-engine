#ifndef UI_HUD2_TOAST_H
#define UI_HUD2_TOAST_H

#include "hud2_batch.h"
#include "hud2_color.h"
#include "../../world/block.h"

// transient corner notifications ("+3 cobble", "saved world", achievement-ish
// pops). a fixed ring of slots, each running a little life state machine:
// rise -> hold -> fall. new toasts push in from the top-right and the stack
// slides to make room. the text itself is drawn by the orchestrator (we only
// own geometry + state), same split as the hotbar label.

#define HUD2_TOAST_MAX     6     // visible at once
#define HUD2_TOAST_QUEUE  24     // pending backlog
#define HUD2_TOAST_TEXT   48     // max chars

typedef enum {
    HUD2_TOAST_FREE = 0,
    HUD2_TOAST_RISE,
    HUD2_TOAST_HOLD,
    HUD2_TOAST_FALL,
} hud2_toast_phase;

typedef enum {
    HUD2_TOAST_INFO = 0,    // neutral
    HUD2_TOAST_GOOD,        // green accent (pickups, success)
    HUD2_TOAST_WARN,        // amber accent
    HUD2_TOAST_BAD,         // red accent (damage, errors)
} hud2_toast_kind;

typedef struct {
    char             text[HUD2_TOAST_TEXT];
    hud2_toast_kind  kind;
    block_id         icon;          // BLOCK_AIR for none
    hud2_toast_phase phase;
    float            t;             // phase-local timer (seconds)
    float            life;          // total hold seconds requested
    float            y;             // smoothed y offset (px), animates on shift
    float            target_y;      // where this slot wants to be
    int              priority;      // higher = survives eviction
} hud2_toast;

typedef struct {
    hud2_toast slots[HUD2_TOAST_MAX];

    // pending queue (ring buffer) for when all slots are busy.
    hud2_toast queue[HUD2_TOAST_QUEUE];
    int        q_head, q_tail, q_count;

    int        sw, sh;
} hud2_toast_stack;

void hud2_toast_init(hud2_toast_stack *ts);

// queue a toast. text is copied (truncated). icon optional. returns 0 if the
// backlog is full and it had to drop a low-priority one.
int  hud2_toast_push(hud2_toast_stack *ts, const char *text,
                     hud2_toast_kind kind, block_id icon, float hold);

// convenience: a block-pickup toast ("+N name").
void hud2_toast_pickup(hud2_toast_stack *ts, block_id id, int amount);

void hud2_toast_update(hud2_toast_stack *ts, int sw, int sh, float dt);
void hud2_toast_draw(hud2_toast_stack *ts, hud2_batch *b);

// per-visible-toast text query for the orchestrator. iterate index 0..MAX-1;
// returns 1 and fills out_* if that slot is live and visible.
int  hud2_toast_text_at(const hud2_toast_stack *ts, int i,
                        const char **out_text, float *out_x, float *out_y,
                        float *out_alpha);

#endif
