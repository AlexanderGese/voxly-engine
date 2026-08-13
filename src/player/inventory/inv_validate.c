#include "inv_validate.h"
#include "inv_stack.h"
#include "inv_registry.h"

inv_verr inv_validate_slot(const inv_stack *s) {
    int has_id    = s->id != INV_ITEM_NONE;
    int has_count = s->count != 0;

    if (has_id && !has_count) return INV_VERR_GHOST_ID;
    if (!has_id && has_count)  return INV_VERR_ORPHAN_COUNT;
    if (!has_id && !has_count) return INV_VERR_NONE;   // a clean empty

    // occupied: id must resolve and the count must fit its cap.
    if (s->id >= (inv_item_id)inv_registry_count()) return INV_VERR_BAD_ID;
    if (s->count > inv_item_max_stack(s->id))        return INV_VERR_OVERFULL;
    return INV_VERR_NONE;
}

int inv_validate_grid(const inv_grid *g, int *first) {
    int bad = 0;
    int firsti = -1;
    for (int i = 0; i < g->count; i++) {
        if (inv_validate_slot(&g->slots[i]) != INV_VERR_NONE) {
            if (firsti < 0) firsti = i;
            bad++;
        }
    }
    if (first) *first = firsti;
    return bad;
}

int inv_validate_repair(inv_grid *g) {
    int touched = 0;
    for (int i = 0; i < g->count; i++) {
        inv_stack *s = &g->slots[i];
        switch (inv_validate_slot(s)) {
        case INV_VERR_NONE:
            break;
        case INV_VERR_GHOST_ID:
            // id with no count: it's really empty, clear the id.
            s->id = INV_ITEM_NONE;
            touched++;
            break;
        case INV_VERR_ORPHAN_COUNT:
            // count with no id: we have no idea what it was, drop it.
            s->count = 0;
            touched++;
            break;
        case INV_VERR_BAD_ID:
            // dangling id from a stale save: wipe the whole slot.
            s->id    = INV_ITEM_NONE;
            s->count = 0;
            touched++;
            break;
        case INV_VERR_OVERFULL:
            // clamp down to the item's current max. lost overflow is gone, but a
            // sane stack beats an exploit that survives across builds.
            s->count = inv_item_max_stack(s->id);
            touched++;
            break;
        }
    }
    return touched;
}

const char *inv_validate_err_name(inv_verr e) {
    switch (e) {
    case INV_VERR_NONE:         return "ok";
    case INV_VERR_GHOST_ID:     return "ghost-id";
    case INV_VERR_ORPHAN_COUNT: return "orphan-count";
    case INV_VERR_OVERFULL:     return "overfull";
    case INV_VERR_BAD_ID:       return "bad-id";
    }
    return "?";
}
