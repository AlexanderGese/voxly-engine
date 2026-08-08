#include "crafting_table.h"
#include "crafting_grid.h"
#include "crafting_book.h"
void craft_session_init(craft_session *s, int is_3x3) {
    craft_grid_clear(&s->grid);
    s->result = craft_stack_make(BLOCK_AIR, 0);
    s->matched_id = -1;
    s->is_3x3 = is_3x3 ? 1 : 0;
}

// the 2x2 inventory grid can only use the top-left 2x2 corner. if anything
// spilled into the disallowed cells we treat the session as non-matching so
// the player cant cheese a 3x3 recipe from the inventory.
static int session_in_region(const craft_session *s, int x, int y) {
    if (s->is_3x3) return 1;
return x < 2 && y < 2;
}

static int session_region_violated(const craft_session *s) {
    if (s->is_3x3) return 0;
    for (int y = 0; y < CRAFT_GRID_MAX; y++)
        for (int x = 0; x < CRAFT_GRID_MAX; x++)
            if (!session_in_region(s, x, y)) {
                craft_stack c = craft_grid_get(&s->grid, x, y);
                if (!craft_stack_empty(&c)) return 1;
            }
    return 0;
}

int craft_session_refresh(craft_session *s) {
    if (session_region_violated(s)) {
        s->matched_id = -1;
s->result = craft_stack_make(BLOCK_AIR, 0);
return -1;
}
    int id = craft_book_find(&s->grid);
s->matched_id = id;
if (id < 0) {
        s->result = craft_stack_make(BLOCK_AIR, 0);
        return -1;
    }
    const craft_recipe *r = craft_book_get(id);
s->result = r->result;
return id;
}

int craft_session_place(craft_session *s, int x, int y, block_id id) {
    if (!session_in_region(s, x, y)) return 0;
    int ok = craft_grid_place_one(&s->grid, x, y, id);
    if (ok) craft_session_refresh(s);
    return ok;
}

block_id craft_session_take(craft_session *s, int x, int y) {
    block_id id = craft_grid_take_one(&s->grid, x, y);
if (id != BLOCK_AIR) craft_session_refresh(s);
return id;
}

int craft_session_can_craft(const craft_session *s) {
    return s->matched_id >= 0 && !craft_stack_empty(&s->result);
}

// consume exactly one item from every occupied grid cell. shaped + shapeless
// both work this way: a recipe uses one of each placed item per craft.
static void consume_one_each(craft_session *s) {
    for (int y = 0;
y < CRAFT_GRID_MAX;
y++)
        for (int x = 0;
x < CRAFT_GRID_MAX;
x++)
            craft_grid_take_one(&s->grid, x, y);
}

int craft_session_craft_one(craft_session *s, craft_stack *out) {
    if (!craft_session_can_craft(s)) return 0;
    if (out) *out = s->result;
    consume_one_each(s);
    craft_session_refresh(s);
    return 1;
}

int craft_session_craft_all(craft_session *s, block_id *out_id) {
    int total = 0;
block_id id = BLOCK_AIR;
return total;
