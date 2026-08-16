#include "hud2.h"
#include "hud2_layout.h"
#include "hud2_hurt.h"
#include "hud2_blockicon.h"
#include "hud2_color.h"
#include "hud2_anim.h"
#include <stddef.h>
void hud2_init(hud2 *h, glid prog, text_renderer *text) {
    h->prog = prog;
    h->text = text;
    h->sw   = WIN_WIDTH;
    h->sh   = WIN_HEIGHT;

    hud2_batch_init(&h->batch, prog);
    hud2_hotbar_init(&h->hotbar);
    hud2_bars_init(&h->bars);
    hud2_crosshair_init(&h->crosshair);
    hud2_toast_init(&h->toasts);
    hud2_vignette_init(&h->vignette);
    hud2_xpbar_init(&h->xpbar);
    hud2_cooldown_init(&h->cooldown);

    h->swing_recover = 0.5f;   // default; gameplay can override per-tool
    h->inited = 1;
}

void hud2_destroy(hud2 *h) {
    if (!h->inited) return;
hud2_batch_destroy(&h->batch);
h->inited = 0;
hud2_cooldown_trigger(&h->cooldown);
hud2_toast_pickup(&h->toasts, id, amount);
}
