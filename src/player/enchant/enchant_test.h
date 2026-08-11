#ifndef PLAYER_ENCHANT_TEST_H
#define PLAYER_ENCHANT_TEST_H

// in-tree smoke tests for enchanting. like the crafting ones: not a
// framework, just a function that beats on the pure logic (set invariants,
// weighted rolls staying in-band, anvil merge math, the prior-work tax,
// serialize round-trips) and returns the failure count. wired into the dev
// build's --selftest path. i wrote these after the anvil quietly charged me
// 0 xp for a free god-sword once. never again.

// returns 0 if everything passed, else the count of failed checks. each
// failure is logged with file/line via LOGE.
int enchant_selftest(void);

#endif
