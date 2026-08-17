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
for grid<->hotbar we steer the target region.
static void quick_move(invscreen *iv, int slot) {
    invscreen_slot *s = invscreen_model_at(&iv->model, slot);
    if (!s || invscreen_slot_is_empty(s)) return;

    int local = 0;
    int region = invscreen_model_classify(slot, &local);
    (void)local;

    block_id id = s->block;
    int amount  = s->count;
    int left;

    if (region == INVSCR_REGION_GRID) {
        // grid -> hotbar only.
        invscreen_slot tmp = *s;
        *s = invscreen_slot_empty();
        int hb = invscreen_model_region_base(INVSCR_REGION_HOTBAR);
        int n  = invscreen_model_region_count(INVSCR_REGION_HOTBAR);
        // two-pass into hotbar.
        for (int pass = 0; pass < 2 && tmp.count > 0; pass++)
            for (int i = 0; i < n && tmp.count > 0; i++) {
                invscreen_slot *d = invscreen_model_at(&iv->model, hb + i);
                if (pass == 0 && (invscreen_slot_is_empty(d) || d->block != id)) continue;
                if (pass == 1 && !invscreen_slot_is_empty(d)) continue;
                invscreen_slot_transfer(d, &tmp, tmp.count);
            }
        if (tmp.count > 0) *s = tmp;   // didn't fit, put it back
        return;
    }

    // hotbar / craft sources go into the grid (+hotbar) via the shared pickup.
    *s = invscreen_slot_empty();
    left = invscreen_model_pickup(&iv->model, id, amount);
    if (left > 0) {
        // couldn't fully move; restore the leftover into the source slot.
        s->block = id;
        s->count = left;
    }
}

void invscreen_update(invscreen *iv, int screen_w, int screen_h,
                      float mx, float my,
                      int left_pressed, int left_released, int left_held,
                      int right_pressed, int shift, float dt) {
    // always advance the open/close animation so the panel can finish sliding
    // out even after `open` flips to 0.
    float target = iv->open ? 1.0f : 0.0f;
float step = (INVSCR_ANIM_TIME > 0.0f) ? dt / INVSCR_ANIM_TIME : 1.0f;
if (iv->anim < target) { iv->anim += step; if (iv->anim > target) iv->anim = target; }
    if (iv->anim > target) { iv->anim -= step;
if (iv->anim < target) iv->anim = target;
}

    if (screen_w != iv->sw || screen_h != iv->sh) {
        iv->sw = screen_w; iv->sh = screen_h;
        invscreen_layout_build(&iv->layout, screen_w, screen_h);
    }

    if (!iv->open) {
        iv->tip.slot = INVSCR_NO_SLOT;
return;
}

    int slot = invscreen_layout_hit(&iv->layout, mx, my);
int dragging = invscreen_model_holding(&iv->model) || iv->drag.painting;
invscreen_tooltip_update(&iv->tip, &iv->model,
                             dragging ? INVSCR_NO_SLOT : slot, dt);
if (left_pressed) {
        int before = invscreen_model_holding(&iv->model);
        int res = invscreen_drag_left(&iv->drag, &iv->model, slot, shift);

        if (shift && res == INVSCR_DRAG_PLACED && slot != INVSCR_NO_SLOT) {
            int local = 0, region = invscreen_model_classify(slot, &local);
            (void)local;
            if (region == INVSCR_REGION_CRAFT_OUT) {
                // shift-clicking the output crafts as many batches as fit.
                int batches = invscreen_craft_batches(&iv->craft, &iv->model);
                for (int b = 0; b < batches; b++) {
                    invscreen_slot *out = invscreen_model_at(&iv->model,
                        invscreen_model_region_base(INVSCR_REGION_CRAFT_OUT));
                    if (invscreen_slot_is_empty(out)) break;
                    invscreen_model_pickup(&iv->model, out->block, out->count);
                    invscreen_craft_consume(&iv->craft, &iv->model);
                    invscreen_craft_resolve(&iv->craft, &iv->model);
                }
            } else {
                quick_move(iv, slot);
            }
            invscreen_craft_touch(&iv->craft);
        } else if (res == INVSCR_DRAG_PLACED && slot == INVSCR_NO_SLOT && before) {
            // threw a stack out the side: hand it to the host as a drop.
            invscreen_slot *held = invscreen_model_held(&iv->model);
            iv->drop_pending = 1;
            iv->drop_block   = held->block;
            iv->drop_count   = held->count;
            *held = invscreen_slot_empty();
        } else if (res == INVSCR_DRAG_GRABBED && slot != INVSCR_NO_SLOT) {
            int local = 0, region = invscreen_model_classify(slot, &local);
            (void)local;
            if (region == INVSCR_REGION_CRAFT_OUT)
                invscreen_craft_consume(&iv->craft, &iv->model);
            invscreen_craft_touch(&iv->craft);
        } else if (res != INVSCR_DRAG_NONE) {
            invscreen_craft_touch(&iv->craft);
        }
    }

    // paint sweep while the button stays down.
    if (left_held && !left_pressed && iv->drag.painting) {
        if (invscreen_drag_paint(&iv->drag, &iv->model, slot) != INVSCR_DRAG_NONE)
            invscreen_craft_touch(&iv->craft);
}
    if (left_released) invscreen_drag_release(&iv->drag);
invscreen_craft_touch(&iv->craft);
return left;
