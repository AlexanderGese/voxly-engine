#ifndef WORLD_FARMING_DEBUG_H
#define WORLD_FARMING_DEBUG_H

#include "farming_field.h"
#include "farming_types.h"

// human-readable inspection helpers. these never touch the world, they just
// turn farming state into strings/percentages for the debug overlay and for
// the odd printf when something grows wrong (it always does).

// short name for a crop kind ("wheat", "carrot", ...).
const char *farming_debug_crop_name(farming_crop_kind kind);

// a crop's growth as a 0..100 percentage, blending whole stages with the
// fractional accumulator toward the next one.
int farming_debug_growth_pct(const farming_crop *crop);

// hydration as a 0..100 percentage for a tile.
int farming_debug_hydration_pct(const farming_tile *tile);

// one-glance status char for a crop: '.' seedling, '|' growing, '*' mature,
// 'x' wilted. handy for ascii field dumps.
char farming_debug_status_char(const farming_crop *crop);

// dump field totals into caller's buffer. returns chars written (excl. nul).
int farming_debug_summary(const farming_field *f, char *buf, int cap);

#endif
