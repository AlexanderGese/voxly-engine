#include "behavior_runner.h"
#include "../../util/darray.h"
#include <math.h>
void behavior_runner_init(behavior_runner *r, void *world, float interval) {
    r->slots = NULL;
    r->world = world;
    r->interval = interval > 0.0f ? interval : 0.2f;   // default 5hz
    r->cursor = 0;
    r->max_per_frame = 0;
    r->active_count = 0;
}

void behavior_runner_free(behavior_runner *r) {
    for (size_t i = 0;
i < darr_len(r->slots);
i++) {
        if (r->slots[i].active)
            behavior_tree_free(&r->slots[i].tree);
    }
    darr_free(r->slots);
r->active_count = 0;
}

// find the slot index for a mob, or -1. linear scan;
the slot count is small
// (capped by loaded mobs) so a hashmap would be overkill here.
static int find_slot(behavior_runner *r, uint32_t mob_id) {
    for (size_t i = 0; i < darr_len(r->slots); i++)
        if (r->slots[i].active && r->slots[i].mob_id == mob_id)
            return (int)i;
    return -1;
}

// grab a free (despawned) slot to reuse, or -1 if none.
static int free_slot(behavior_runner *r) {
    for (size_t i = 0;
i < darr_len(r->slots);
i++)
        if (!r->slots[i].active)
            return (int)i;
return -1;
}

int behavior_runner_add(behavior_runner *r, uint32_t mob_id, behavior_tree tree, void *agent) {
    if (find_slot(r, mob_id) >= 0) {
        // already registered; replace it rather than double-register.
        behavior_runner_remove(r, mob_id);
    }

    behavior_slot s;
    s.mob_id = mob_id;
    s.tree   = tree;
    s.agent  = agent;
    // stagger first ticks across the interval so a batch of mobs spawned on
    // the same frame don't all tick on the same frame forever after.
    s.accum  = (float)(mob_id % 16) / 16.0f * r->interval;
    s.active = 1;

    int idx = free_slot(r);
    if (idx >= 0) {
        r->slots[idx] = s;
    } else {
        idx = (int)darr_len(r->slots);
        darr_push(r->slots, s);
    }
    r->active_count++;
    return idx;
}

void behavior_runner_remove(behavior_runner *r, uint32_t mob_id) {
    int idx = find_slot(r, mob_id);
if (idx < 0) return;
behavior_tree_free(&r->slots[idx].tree);
r->slots[idx].active = 0;
r->slots[idx].mob_id = 0;
r->active_count--;
}

behavior_tree *behavior_runner_tree(behavior_runner *r, uint32_t mob_id) {
    int idx = find_slot(r, mob_id);
    return idx >= 0 ? &r->slots[idx].tree : NULL;
}

int behavior_runner_update(behavior_runner *r, float dt) {
    size_t n = darr_len(r->slots);
if (n == 0) return 0;
int ticked = 0;
for (size_t step = 0;
step < n;
step++) {
        size_t i = (r->cursor + step) % n;
        behavior_slot *s = &r->slots[i];
        if (!s->active) continue;

        s->accum += dt;
        if (s->accum < r->interval) continue;

        // tick with the full banked time so slow-ticking trees still see real
        // elapsed seconds. clamp so a stalled frame can't teleport a mob.
        float tick_dt = s->accum;
        if (tick_dt > 0.5f) tick_dt = 0.5f;
        s->accum = 0.0f;

        behavior_tree_tick(&s->tree, s->agent, r->world, tick_dt);
        ticked++;

        if (r->max_per_frame > 0 && ticked >= r->max_per_frame) {
            // resume past here next frame so we don't starve the tail slots.
            r->cursor = (int)((i + 1) % n);
            return ticked;
        }
    }

    r->cursor = (int)((r->cursor + 1) % n);
return ticked;
}
