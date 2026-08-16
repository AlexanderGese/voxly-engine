#ifndef UI_HUD2_H
#define UI_HUD2_H
#include "../../render/gl.h"
#include "../../render/text.h"
#include "../../player/inventory.h"
#include "../../game/health.h"
#include "../../math/vec3.h"
#include "hud2_batch.h"
#include "hud2_hotbar.h"
#include "hud2_bars.h"
#include "hud2_crosshair.h"
#include "hud2_toast.h"
#include "hud2_vignette.h"
#include "hud2_xpbar.h"
#include "hud2_cooldown.h"
// hud2: the second-gen heads-up display. owns the shared 2d batcher and all
// the widgets, drives their animations, and composites them in the right
typedef struct {
    hud2_batch       batch;
    text_renderer   *text;      // borrowed, not owned. orchestrator-supplied.

    hud2_hotbar      hotbar;
    hud2_bars        bars;
    hud2_crosshair   crosshair;
    hud2_toast_stack toasts;
    hud2_vignette    vignette;
    hud2_xpbar       xpbar;
    hud2_cooldown    cooldown;

    float            swing_recover;   // seconds for the cooldown arc to refill

    glid  prog;                 // the hud2 2d color shader
    int   inited;
    int   sw, sh;               // last known screen size
} hud2;
void hud2_init(hud2 *h, glid prog, text_renderer *text);
void hud2_destroy(hud2 *h);
void hud2_update(hud2 *h, const inventory *inv, const survival *s,
                 int targeting, int sw, int sh, float dt);
void hud2_set_xp(hud2 *h, int total_xp);
void hud2_render(hud2 *h, const inventory *inv, const survival *s);
void hud2_on_swing(hud2 *h);
void hud2_on_hit_confirm(hud2 *h);
void hud2_on_pickup(hud2 *h, block_id id, int amount);
void hud2_on_damage(hud2 *h, int amount,
                    vec3 player_pos, float yaw, vec3 source);
void hud2_notify(hud2 *h, const char *text, hud2_toast_kind kind);
#endif
