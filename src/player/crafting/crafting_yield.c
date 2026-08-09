#include "crafting_yield.h"
#include "crafting_grid.h"
#include "crafting_remainder.h"
#include "crafting_stats.h"
#include <string.h>

// does any occupied cell hold an item with a remainder mapping? if so we have
// to route consumption through craft_remainder_apply instead of the plain
// one-off-each decrement the table does.
static int grid_has_remainder(const craft_grid *g) {
    for (int i = 0; i < CRAFT_GRID_CELLS; i++) {
        const craft_stack *c = &g->cell[i];
        if (craft_stack_empty(c)) continue;
        if (craft_remainder_has(c->id)) return 1;
    }
    return 0;
}

// consume one of each occupied cell, honouring remainders. mirrors the table's
// consume_one_each but lets byproducts stick around.
static void consume_with_remainder(craft_session *s) {
    if (grid_has_remainder(&s->grid)) {
        craft_remainder_apply(&s->grid);
        return;
    }
    for (int y = 0; y < CRAFT_GRID_MAX; y++)
        for (int x = 0; x < CRAFT_GRID_MAX; x++)
            craft_grid_take_one(&s->grid, x, y);
}

int craft_yield_once(craft_session *s, craft_stack *out) {
    if (!craft_session_can_craft(s)) return 0;
    int rid = s->matched_id;
    craft_stack result = s->result;
    if (out) *out = result;

    consume_with_remainder(s);
    craft_session_refresh(s);

    craft_stats_record(rid, result.count);
    return 1;
}

int craft_yield_all(craft_session *s, block_id *out_id) {
    int total = 0;
    block_id id = BLOCK_AIR;
    while (craft_session_can_craft(s)) {
        craft_stack r = s->result;
        if (id == BLOCK_AIR) id = r.id;
        else if (r.id != id) break;       // a different recipe slid into place
        craft_stack got;
        if (!craft_yield_once(s, &got)) break;
        total += got.count;
    }
    if (out_id) *out_id = id;
    return total;
}

int craft_yield_potential(const craft_session *s) {
    // work on a copy so the live session is untouched. craft_session is a flat
    // value type (grid + a few ints) so a struct copy is all we need.
    craft_session tmp = *s;
    int crafts = 0;
    while (craft_session_can_craft(&tmp)) {
        block_id before = tmp.result.id;
        consume_with_remainder(&tmp);
        craft_session_refresh(&tmp);
        crafts++;
        // safety valve: a self-remainder recipe (item survives) could loop
        // forever since the grid never empties. bail if nothing actually
        // changed in the matched result and the grid still looks identical.
        if (tmp.result.id == before && craft_grid_is_empty(&tmp.grid) == 0
            && crafts > CRAFT_MAX_RESULT)
            break;
    }
    return crafts;
}
