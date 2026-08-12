#include "inv_hotbar.h"
#include "inv_stack.h"
#include "inv_registry.h"
#include <stddef.h>

void inv_hotbar_bind(inv_hotbar *h, inv_player *p) {
    h->p = p;
}

inv_stack *inv_hotbar_slot(inv_hotbar *h, int idx) {
    if (!h->p || idx < 0 || idx >= INV_HOTBAR_SLOTS) return NULL;
    return inv_grid_at(&h->p->bag, INV_MAIN_SLOTS + idx);
}

const inv_stack *inv_hotbar_cslot(const inv_hotbar *h, int idx) {
    if (!h->p || idx < 0 || idx >= INV_HOTBAR_SLOTS) return NULL;
    return inv_grid_cat(&h->p->bag, INV_MAIN_SLOTS + idx);
}

inv_stack *inv_hotbar_selected(inv_hotbar *h) {
    if (!h->p) return NULL;
    return inv_hotbar_slot(h, h->p->selected);
}

block_id inv_hotbar_place_block(const inv_hotbar *h) {
    const inv_stack *s = inv_hotbar_cslot(h, h->p ? h->p->selected : -1);
    if (!s || inv_stack_is_empty(s)) return BLOCK_AIR;
    return inv_registry_get(s->id)->place_block;
}

int inv_hotbar_can_place(const inv_hotbar *h) {
    return inv_hotbar_place_block(h) != BLOCK_AIR;
}

void inv_hotbar_cycle(inv_hotbar *h, int delta) {
    if (h->p) inv_player_scroll(h->p, delta);
}

int inv_hotbar_selected_index(const inv_hotbar *h) {
    return h->p ? h->p->selected : 0;
}
