#ifndef PLAYER_INVENTORY_TEST_H
#define PLAYER_INVENTORY_TEST_H

// in-tree smoke tests for the inventory subsystem. same deal as crafting_test:
// not a framework, just a function that hammers the pure bits (stack merge/split,
// grid add overflow, transfers, the undo journal, validate/repair, save round
// trip) and returns the failure count. wired into the dev `--selftest` path.
//
// every one of these caught a real bug at least once. the journal ring math
// especially — undo groups across a wrap are exactly the kind of off-by-one i'd
// ship without this.

// returns 0 if everything passed, else the count of failed checks. each failure
// is logged with file/line via LOGE.
int inventory_selftest(void);

#endif
