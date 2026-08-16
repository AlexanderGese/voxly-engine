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
#endif
