#include "fishing_stats.h"
#include <stddef.h>
#include <string.h>

void fishing_stats_init(fishing_stats *s) {
    memset(s, 0, sizeof *s);
}

void fishing_stats_on_cast(fishing_stats *s) {
    s->casts++;
}

void fishing_stats_on_miss(fishing_stats *s) {
    s->misses++;
}

void fishing_stats_on_snap(fishing_stats *s) {
    s->snaps++;
}

void fishing_stats_on_catch(fishing_stats *s, const fishing_catch *c) {
    if (!c || c->category == CATCH_NONE) return;

    s->catches++;
    s->total_items += c->count;
    if (c->category >= 0 && c->category < CATCH_CATEGORY_COUNT)
        s->per_category[c->category]++;

    // push into the ring, oldest gets overwritten once it wraps.
    fishing_stats_entry *e = &s->history[s->head];
    e->category = c->category;
    e->block    = c->block;
    e->count    = c->count;

    s->head = (s->head + 1) % FISHING_HISTORY_LEN;
    if (s->filled < FISHING_HISTORY_LEN) s->filled++;
}

float fishing_stats_catch_rate(const fishing_stats *s) {
    if (s->casts <= 0) return 0.0f;
    float r = (float)s->catches / (float)s->casts;
    return r > 1.0f ? 1.0f : r;   // cancelled casts can't push this over 1
}

int fishing_stats_recent(const fishing_stats *s, int back, fishing_stats_entry *out) {
    // back=0 is the most recent catch, back=1 the one before, etc.
    if (!out || back < 0 || back >= s->filled) return 0;

    // head points one past the newest; walk backwards from there.
    int idx = s->head - 1 - back;
    while (idx < 0) idx += FISHING_HISTORY_LEN;

    *out = s->history[idx];
    return 1;
}
