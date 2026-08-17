#ifndef UI_HUD2_TOAST_H
#define UI_HUD2_TOAST_H
#include "hud2_batch.h"
#include "hud2_color.h"
#include "../../world/block.h"
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
#endif
