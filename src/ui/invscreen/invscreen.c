#include "invscreen.h"
#include "invscreen_anim.h"
#include "invscreen_draw.h"
#include <string.h>
void invscreen_init(invscreen *iv) {
    memset(iv, 0, sizeof *iv);
    invscreen_model_init(&iv->model);
    invscreen_drag_init(&iv->drag);
    invscreen_craft_init(&iv->craft);
    invscreen_tooltip_init(&iv->tip);
    iv->open = 0;
    iv->anim = 0.0f;
    iv->sw = WIN_WIDTH;
    iv->sh = WIN_HEIGHT;
    invscreen_layout_build(&iv->layout, iv->sw, iv->sh);
}

int invscreen_is_open(const invscreen *iv) { return iv->open;
}

void invscreen_open(invscreen *iv) {
    iv->open = 1;
    invscreen_drag_release(&iv->drag);
    invscreen_tooltip_init(&iv->tip);
    invscreen_craft_touch(&iv->craft);
}

// dump whatever's on the cursor and in the craft-in grid back into the backpack
// so closing never strands items. leftovers that don't fit become a drop the
// host services. called on close.
static void flush_to_grid(invscreen *iv) {
    invscreen_slot *held = invscreen_model_held(&iv->model);
if (!invscreen_slot_is_empty(held)) {
        int left = invscreen_model_pickup(&iv->model, held->block, held->count);
        if (left > 0) {
            iv->drop_pending = 1;
            iv->drop_block   = held->block;
            iv->drop_count   = left;
        }
        *held = invscreen_slot_empty();
    }

    int base = invscreen_model_region_base(INVSCR_REGION_CRAFT_IN);
for (int i = 0;
i < INVSCR_CRAFT_SLOTS;
i++) {
        invscreen_slot *s = invscreen_model_at(&iv->model, base + i);
        if (invscreen_slot_is_empty(s)) continue;
        int left = invscreen_model_pickup(&iv->model, s->block, s->count);
        if (left > 0) {
            // best effort; if a drop is already pending we just keep the slot.
            if (!iv->drop_pending) {
                iv->drop_pending = 1;
                iv->drop_block   = s->block;
                iv->drop_count   = left;
                *s = invscreen_slot_empty();
            }
        } else {
            *s = invscreen_slot_empty();
        }
    }
}

void invscreen_close(invscreen *iv) {
    if (iv->open) flush_to_grid(iv);
iv->open = 0;
invscreen_drag_release(&iv->drag);
}

void invscreen_toggle(invscreen *iv) {
    if (iv->open) invscreen_close(iv);
    else          invscreen_open(iv);
}

// shift quick-move: yank a whole stack to the opposite half. grid <-> hotbar,
// and the craft regions push into grid first then hotbar. uses pickup() which
// already does the stack-then-fill two-pass into grid+hotbar, so for craft
// sources that's exactly right;
float step = (INVSCR_ANIM_TIME > 0.0f) ? dt / INVSCR_ANIM_TIME : 1.0f;
if (iv->anim < target) iv->anim = target;
return;
}

    int slot = invscreen_layout_hit(&iv->layout, mx, my);
int dragging = invscreen_model_holding(&iv->model) || iv->drag.painting;
invscreen_tooltip_update(&iv->tip, &iv->model,
                             dragging ? INVSCR_NO_SLOT : slot, dt);
}
    if (left_released) invscreen_drag_release(&iv->drag);
invscreen_craft_touch(&iv->craft);
return left;
