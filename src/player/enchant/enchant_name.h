#ifndef PLAYER_ENCHANT_NAME_H
#define PLAYER_ENCHANT_NAME_H

#include "enchant_types.h"
#include <stddef.h>
#include <stdint.h>

// the flavor text. two jobs:
// 1. the human-readable enchant label ("sharpness iv"), built from the
// registry name plus a roman numeral.
// 2. the spooky "standard galactic alphabet" gibberish the table shows over
// an offer before you take it, generated deterministically from the seed
// so it's stable while the offer is.
//
// neither allocates; you pass a buffer and we fill it.

// write "name level" (e.g. "efficiency iii") into buf. level 1 is rendered
// without a numeral for the single-level enchants. returns chars written
// (excluding the nul), or 0 on a bad id.
size_t enchant_name_label(enchant_id id, int level, char *buf, size_t cap);

// roman numeral for 1..10 into buf (needs >= 5 bytes). returns length.
size_t enchant_name_roman(int n, char *buf, size_t cap);

// fill buf with `glyphs` pseudo-random galactic glyphs (rendered here as the
// ascii letters the atlas maps to that font row), seeded by `seed`. always
// nul-terminates. returns chars written.
size_t enchant_name_galactic(uint64_t seed, int glyphs, char *buf, size_t cap);

#endif
