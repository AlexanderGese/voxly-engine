#include "tools_loadout.h"
#include "tools_speed.h"

void tools_loadout_init(tools_loadout *L) {
    L->hand = tools_item_hand();
    for (int i = 0; i < HOTBAR_SLOTS; i++) {
        L->slot[i] = L->hand;
        L->has[i]  = 0;
    }
    L->selected = 0;
}

void tools_loadout_set(tools_loadout *L, int slot, tool_item t) {
    if (slot < 0 || slot >= HOTBAR_SLOTS) return;
    if (t.head.kind == TOOL_HAND) { tools_loadout_clear(L, slot); return; }
    L->slot[slot] = t;
    L->has[slot]  = 1;
}

void tools_loadout_clear(tools_loadout *L, int slot) {
    if (slot < 0 || slot >= HOTBAR_SLOTS) return;
    L->slot[slot] = L->hand;
    L->has[slot]  = 0;
}

void tools_loadout_select(tools_loadout *L, int slot) {
    if (slot < 0 || slot >= HOTBAR_SLOTS) return;
    L->selected = slot;
}

tool_item *tools_loadout_current(tools_loadout *L) {
    int s = L->selected;
    if (s < 0 || s >= HOTBAR_SLOTS) return &L->hand;
    // a broken tool still occupies the slot but digs like the hand. return it
    // anyway so the ui can show the snapped tool; the dig math handles broken.
    if (!L->has[s]) return &L->hand;
    return &L->slot[s];
}

// quick env-free break time used purely for comparing tools against each other.
// we don't thread the real dig_env through here on purpose: relative ordering
// of tools doesn't change with haste/fatigue, so a flat env is fine and cheap.
static float quick_break_time(const tool_item *t, block_id block) {
    dig_env e = tools_env_default();
    return tools_speed_break_time(t, block, &e);
}

int tools_loadout_best_for(tools_loadout *L, block_id block) {
    int   best     = -1;
    float best_time = quick_break_time(&L->hand, block);
    int   best_harv = tools_speed_can_harvest(&L->hand, block);

    for (int i = 0; i < HOTBAR_SLOTS; i++) {
        if (!L->has[i]) continue;
        tool_item *t = &L->slot[i];
        if (tools_item_is_broken(t)) continue;     // snapped, treat as hand

        int   harv = tools_speed_can_harvest(t, block);
        float time = quick_break_time(t, block);

        // a tool that can actually harvest always beats one that can't, even if
        // the non-harvesting one is technically faster. drops > speed.
        if (harv && !best_harv) {
            best = i; best_time = time; best_harv = 1;
            continue;
        }
        if (harv == best_harv && time < best_time - 1e-4f) {
            best = i; best_time = time;
        }
    }
    return best;
}

int tools_loadout_auto_switch(tools_loadout *L, block_id block) {
    int b = tools_loadout_best_for(L, block);
    if (b >= 0) {
        L->selected = b;
        return b;
    }
    // nothing beats the hand. don't force a slot, just leave selection alone if
    // the current pick already holds nothing useful, otherwise note we'd dig
    // bare-handed. callers that want a hard switch can clear selection.
    return -1;
}
