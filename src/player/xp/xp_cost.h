#ifndef PLAYER_XP_XP_COST_H
#define PLAYER_XP_XP_COST_H

// the other half of an economy: spending xp. anvils, enchanting tables and
// repairs all charge *levels*, not raw xp, which is its own little wrinkle —
// you pay by dropping whole levels and forfeiting the partial progress.
//
// these helpers compute costs and apply spends against an xp_state, so the
// curve stays the single source of truth.

#include "xp_state.h"

typedef enum {
    XP_COST_REPAIR,    // anvil repair, scales with prior work count
    XP_COST_ENCHANT,   // enchanting table, scales with the chosen power
    XP_COST_RENAME,    // cheap, flat
    XP_COST_COMBINE,   // combine two items, the spendy one
} xp_cost_kind;

// level cost for an operation. `magnitude` means: prior-work-uses for repair,
// enchant power (1..3) for enchant, item count for combine, ignored for
// rename. returns a level count (>= 0).
int  xp_cost_levels(xp_cost_kind kind, int magnitude);

// can the player afford `levels`?
int  xp_cost_can_afford(const xp_state *s, int levels);

// spend `levels`. drops the player by that many whole levels, snapping their
// total down to the start of the resulting level (partial progress is lost,
// just like the real thing). returns 1 if spent, 0 if too poor.
int  xp_cost_spend(xp_state *s, int levels);

// the "too expensive!" guard: anvils refuse operations costing >= this many
// levels. exposed so ui can grey out the button before committing.
#define XP_COST_TOO_EXPENSIVE 40

#endif
