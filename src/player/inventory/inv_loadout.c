#include "inv_loadout.h"
#include "inv_stack.h"
#include <string.h>

void inv_loadout_bank_init(inv_loadout_bank *bank) {
    memset(bank, 0, sizeof *bank);   // used=0 everywhere == all empty
}

static int idx_ok(int idx) {
    return idx >= 0 && idx < INV_LOADOUT_MAX;
}

int inv_loadout_save(inv_loadout_bank *bank, int idx, const char *name,
                     const inv_player *p) {
    if (!idx_ok(idx)) return -1;
    inv_loadout *L = &bank->bank[idx];

    // copy the name in, truncated and always terminated.
    if (name) {
        size_t n = strlen(name);
        if (n >= INV_LOADOUT_NAMELEN) n = INV_LOADOUT_NAMELEN - 1;
        memcpy(L->name, name, n);
        L->name[n] = '\0';
    } else {
        L->name[0] = '\0';
    }

    // snapshot the hotbar tail of the bag: ids only.
    for (int i = 0; i < INV_HOTBAR_SLOTS; i++) {
        const inv_stack *s = inv_grid_cat(&p->bag, INV_MAIN_SLOTS + i);
        L->slot[i] = (s && !inv_stack_is_empty(s)) ? s->id : INV_ITEM_NONE;
    }
    L->selected = p->selected;
    L->used     = 1;
    return 0;
}

// find a bag slot (anywhere) holding `id` other than `except`. -1 if none.
static int find_holding(inv_player *p, inv_item_id id, int except) {
    if (id == INV_ITEM_NONE) return -1;
    for (int i = 0; i < p->bag.count; i++) {
        if (i == except) continue;
        const inv_stack *s = &p->bag.slots[i];
        if (s->id == id && s->count > 0) return i;
    }
    return -1;
}

int inv_loadout_apply(const inv_loadout_bank *bank, int idx, inv_player *p) {
    if (!idx_ok(idx)) return 0;
    const inv_loadout *L = &bank->bank[idx];
    if (!L->used) return 0;

    int satisfied = 0;
    for (int i = 0; i < INV_HOTBAR_SLOTS; i++) {
        inv_item_id want = L->slot[i];
        int dst = INV_MAIN_SLOTS + i;
        inv_stack *d = &p->bag.slots[dst];

        if (want == INV_ITEM_NONE) {
            // the layout wanted this slot clear; leave whatever's there. clearing
            // would mean dumping items with nowhere to go, which is rude.
            satisfied++;
            continue;
        }
        if (d->id == want && d->count > 0) {
            satisfied++;       // already correct
            continue;
        }
        // pull a stack of the wanted item from elsewhere in the bag and swap it
        // into place. whatever was here goes where that came from.
        int from = find_holding(p, want, dst);
        if (from < 0) continue;     // we just don't have it; ui can warn
        inv_stack_swap(d, &p->bag.slots[from]);
        satisfied++;
    }

    if (L->selected >= 0 && L->selected < INV_HOTBAR_SLOTS)
        p->selected = L->selected;
    return satisfied;
}

int inv_loadout_clear(inv_loadout_bank *bank, int idx) {
    if (!idx_ok(idx)) return -1;
    memset(&bank->bank[idx], 0, sizeof bank->bank[idx]);
    return 0;
}

const inv_loadout *inv_loadout_peek(const inv_loadout_bank *bank, int idx) {
    if (!idx_ok(idx)) return NULL;
    const inv_loadout *L = &bank->bank[idx];
    return L->used ? L : NULL;
}
