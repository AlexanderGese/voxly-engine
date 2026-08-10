#include "enchant_set.h"
#include "enchant_registry.h"

#include <stddef.h>
#include <string.h>

void enchant_set_clear(enchant_set *s) {
    if (!s) return;
    memset(s, 0, sizeof *s);
}

int enchant_set_count(const enchant_set *s) {
    return s ? s->count : 0;
}

// binary search for an id in the sorted entry array. returns the index if
// found, otherwise the negative insertion point encoded as -(pos+1) so the
// caller can both detect a miss and learn where to slot a new entry.
static int find_slot(const enchant_set *s, enchant_id id) {
    int lo = 0, hi = s->count - 1;
    while (lo <= hi) {
        int mid = (lo + hi) >> 1;
        enchant_id m = s->entry[mid].id;
        if (m == id) return mid;
        if (m < id) lo = mid + 1;
        else        hi = mid - 1;
    }
    return -(lo + 1);
}

int enchant_set_level(const enchant_set *s, enchant_id id) {
    if (!s || id == ENCHANT_NONE) return 0;
    int idx = find_slot(s, id);
    return idx >= 0 ? s->entry[idx].level : 0;
}

int enchant_set_has(const enchant_set *s, enchant_id id) {
    return enchant_set_level(s, id) > 0;
}

int enchant_set_put(enchant_set *s, enchant_id id, int level) {
    if (!s || id == ENCHANT_NONE) return -1;
    const enchant_def *d = enchant_registry_get(id);
    if (!d) return -1;

    // clamp to both the per-enchant cap and the absolute ceiling.
    if (level > d->max_level) level = d->max_level;
    if (level > ENCHANT_MAX_LEVEL) level = ENCHANT_MAX_LEVEL;
    if (level <= 0) return enchant_set_remove(s, id) ? 1 : 0;

    int idx = find_slot(s, id);
    if (idx >= 0) {
        if (s->entry[idx].level >= (uint8_t)level) return 0; // no downgrade
        s->entry[idx].level = (uint8_t)level;
        return 1;
    }

    if (s->count >= ENCHANT_MAX_ON_ITEM) return -1;

    int pos = -(idx + 1); // insertion point keeps the array sorted
    memmove(&s->entry[pos + 1], &s->entry[pos],
            (size_t)(s->count - pos) * sizeof s->entry[0]);
    s->entry[pos].id    = id;
    s->entry[pos].level = (uint8_t)level;
    s->count++;
    return 1;
}

int enchant_set_remove(enchant_set *s, enchant_id id) {
    if (!s) return 0;
    int idx = find_slot(s, id);
    if (idx < 0) return 0;
    memmove(&s->entry[idx], &s->entry[idx + 1],
            (size_t)(s->count - idx - 1) * sizeof s->entry[0]);
    s->count--;
    s->entry[s->count].id = ENCHANT_NONE;
    s->entry[s->count].level = 0;
    return 1;
}

enchant_id enchant_set_conflict(const enchant_set *s, enchant_id id) {
    const enchant_def *d = enchant_registry_get(id);
    if (!s || !d) return ENCHANT_NONE;
    for (int i = 0; i < 4; ++i) {
        enchant_id c = d->conflicts[i];
        if (c == ENCHANT_NONE) break;
        if (enchant_set_has(s, c)) return c;
    }
    return ENCHANT_NONE;
}

int enchant_set_equal(const enchant_set *a, const enchant_set *b) {
    if (a == b) return 1;
    if (!a || !b) return 0;
    if (a->count != b->count) return 0;
    // both sorted, so a straight memcmp of the live prefix works.
    return memcmp(a->entry, b->entry,
                  (size_t)a->count * sizeof a->entry[0]) == 0;
}
