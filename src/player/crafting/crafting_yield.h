#ifndef PLAYER_CRAFTING_YIELD_H
#define PLAYER_CRAFTING_YIELD_H

#include "crafting_types.h"
#include "crafting_table.h"

// the "do the craft for real" layer that the existing table doesn't cover: it
// routes a craft through remainder handling and records it in the stats. the
// base craft_session_craft_one just decrements cells; this is what the game ui
// actually calls so byproducts stay in the grid and the book learns what you
// make.
//
// kept separate from crafting_table.c so that file stays a dumb state holder
// and the cross-module wiring (remainder + stats) lives in one obvious place.

// craft once from the session: validates, emits the result via *out, applies
// ingredient remainders to the grid, records the craft in stats, and refreshes
// the session preview. returns 1 on success, 0 if nothing matched.
int craft_yield_once(craft_session *s, craft_stack *out);

// craft repeatedly until the grid no longer satisfies the recipe, summing the
// output. writes the produced id to *out_id and returns total items made. each
// iteration honours remainders, so a recipe whose ingredient leaves a leftover
// will naturally stop once the consumable parts run out.
int craft_yield_all(craft_session *s, block_id *out_id);

// preview-only: how many full crafts can the current grid support before it
// stops matching? does NOT mutate the session (works on a copy). useful for
// the "craft x12" tooltip.
int craft_yield_potential(const craft_session *s);

#endif
