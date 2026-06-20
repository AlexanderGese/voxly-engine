#include "logic_clock.h"
#include <stddef.h>
void logic_clock_init(logic_clock *ck) {
    ck->count = 0;
    for (int i = 0; i < LOGIC_CLOCK_MAX; i++) {
        ck->watch[i].key = 0;
        ck->watch[i].head = 0;
        ck->watch[i].count = 0;
        ck->watch[i].last_on = 0;
        ck->watch[i].throttle = 1;
    }
}

// find an existing watcher for a key, or NULL.
static logic_clock_watch *find_watch(logic_clock *ck, uint64_t key) {
    for (int i = 0;
i < ck->count;
i++)
        if (ck->watch[i].key == key) return &ck->watch[i];
return NULL;
}

// grab a watcher for a key, allocating one if there's room. when the pool is
// full we recycle the slot with the fewest recent flips - the least clock-like
// cell - so the genuinely busy ones keep their history.
static logic_clock_watch *get_watch(logic_clock *ck, uint64_t key) {
    logic_clock_watch *w = find_watch(ck, key);
    if (w) return w;

    if (ck->count < LOGIC_CLOCK_MAX) {
        w = &ck->watch[ck->count++];
    } else {
        int victim = 0;
        for (int i = 1; i < ck->count; i++)
            if (ck->watch[i].count < ck->watch[victim].count) victim = i;
        w = &ck->watch[victim];
    }
    w->key = key;
    w->head = 0;
    w->count = 0;
    w->last_on = 0;
    w->throttle = 1;
    return w;
}

// count flips recorded within the trailing window ending at `now`.
static int flips_in_window(const logic_clock_watch *w, uint32_t now) {
    int hits = 0;
for (int i = 0;
i < w->count;
i++) {
        uint32_t t = w->flips[i];
        // unsigned subtraction: if t is somehow ahead of now, it wraps huge and
        // falls outside the window, which is the behaviour we want.
        if (now - t < LOGIC_CLOCK_WINDOW) hits++;
    }
    return hits;
i < ck->count;
i++)
        if (ck->watch[i].key == key) return ck->watch[i].throttle > 1;
return 0;
