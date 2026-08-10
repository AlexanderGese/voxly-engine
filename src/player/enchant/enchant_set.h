#ifndef PLAYER_ENCHANT_SET_H
#define PLAYER_ENCHANT_SET_H

#include "enchant_types.h"

// operations on the per-item enchant_set. the invariant the whole subsystem
// leans on: entries are kept sorted by id and there are no duplicate ids and
// no level-0 entries. every mutator restores that invariant before returning.

void enchant_set_clear(enchant_set *s);

// number of live entries.
int  enchant_set_count(const enchant_set *s);

// level of a given enchant on the item, 0 if absent.
int  enchant_set_level(const enchant_set *s, enchant_id id);

int  enchant_set_has(const enchant_set *s, enchant_id id);

// add or raise an enchant. if already present at >= level, no-op and returns
// 0; otherwise stamps the new level (clamped to the def max) and returns 1.
// fails (returns -1) if the set is full or the id is unknown.
int  enchant_set_put(enchant_set *s, enchant_id id, int level);

// remove an enchant if present. returns 1 if something was removed.
int  enchant_set_remove(enchant_set *s, enchant_id id);

// would adding `id` violate a mutual-exclusion conflict with what's already
// on the item? returns the conflicting id, or ENCHANT_NONE if clear.
enchant_id enchant_set_conflict(const enchant_set *s, enchant_id id);

// two sets equal iff same ids at same levels. relies on the sorted invariant.
int  enchant_set_equal(const enchant_set *a, const enchant_set *b);

#endif
