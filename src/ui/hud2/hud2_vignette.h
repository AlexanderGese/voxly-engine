#ifndef UI_HUD2_VIGNETTE_H
#define UI_HUD2_VIGNETTE_H
#include "hud2_batch.h"
#include "hud2_color.h"
// full-screen edge overlays: a low-health red pulse around the borders, and a
// short directional hurt flash that hits hardest on the side damage came from.
// the batcher only does solid quads, so the "vignette" is faked with a stack
// of edge bands whose alpha falls off toward the center. cheap but reads fine.
#define HUD2_VIG_BANDS  6     // edge bands per side, more = smoother falloff
typedef struct {
    float hurt;          // 0..1 directional flash amount, decays
    float hurt_dir;      // angle of the hit in radians (0 = front/center)
    float low_pulse;     // free-running clock for the low-hp throb
    float low_amt;       // 0..1 strength of the persistent low-hp vignette
    int   inited;
} hud2_vignette;
void hud2_vignette_init(hud2_vignette *v);
// flash from a world-space direction relative to the player's facing. dir is
// the angle in the screen plane: -pi..pi, 0 = ahead. strength 0..1.
void hud2_vignette_hurt(hud2_vignette *v, float dir, float strength);
// feed normalized health each frame; drives the persistent low-hp vignette.
void hud2_vignette_update(hud2_vignette *v, float health01, float dt);
void hud2_vignette_draw(hud2_vignette *v, hud2_batch *b, int sw, int sh);
#endif
