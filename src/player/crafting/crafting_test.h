#ifndef PLAYER_CRAFTING_TEST_H
#define PLAYER_CRAFTING_TEST_H

// in-tree smoke tests for the crafting matcher. not a framework, just a
// function that hammers the pure bits (grid bounds, shaped/shapeless match,
// mirror, yield, region gating) and returns the failure count. wired into the
// dev build's `--selftest` path, no-op everywhere else. saves me every time i
// "just reorder the bbox loop".

// returns 0 if everything passed, else the count of failed checks. each
// failure is logged with file/line via LOGE.
int crafting_selftest(void);

#endif
