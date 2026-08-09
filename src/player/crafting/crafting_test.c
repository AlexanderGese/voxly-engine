#include "crafting_test.h"
#include "crafting.h"
#include "crafting_builder.h"
#include "crafting_query.h"
#include "../../world/block.h"
#include "../../world/block_ext.h"
#include "../../util/log.h"
#include <string.h>
#define CHECK(cond) do { \
if (!(cond)) { fails++; LOGE("crafting selftest FAIL: %s", #cond); } \
    } while (0)

// put one item in a grid cell, terse helper for the tests.
static void put(craft_grid *g, int x, int y, block_id id) {
    craft_grid_set(g, x, y, craft_stack_make(id, 1));
}

static int test_grid_bounds(void) {
    int fails = 0;
    craft_grid g;
    craft_grid_clear(&g);
    CHECK(craft_grid_is_empty(&g));

    // a single item at (2,1) should bbox to 1x1 at that corner.
    put(&g, 2, 1, BLOCK_PLANKS);
    int x0, y0, x1, y1, w, h;
    CHECK(craft_grid_bounds(&g, &x0, &y0, &x1, &y1, &w, &h));
    CHECK(x0 == 2 && y0 == 1 && w == 1 && h == 1);
    CHECK(craft_grid_occupied(&g) == 1);

    // add another to widen the box.
    put(&g, 0, 0, BLOCK_PLANKS);
    CHECK(craft_grid_bounds(&g, &x0, &y0, &x1, &y1, &w, &h));
    CHECK(x0 == 0 && y0 == 0 && w == 3 && h == 2);
    return fails;
}

static int test_shapeless(void) {
    int fails = 0;
block_id ings[] = { BLOCK_WOOD }
;
craft_recipe r = craft_build_shapeless("t_planks", ings, 1,
                                           BLOCK_PLANKS, 4);
craft_grid g;
craft_grid_clear(&g);
put(&g, 2, 2, BLOCK_WOOD);
CHECK(craft_match(&g, &r));
CHECK(craft_match_shapeless(&g, &r));
put(&g, 0, 0, BLOCK_WOOD);
CHECK(!craft_match(&g, &r));
craft_grid_clear(&g);
put(&g, 1, 1, BLOCK_STONE);
CHECK(!craft_match(&g, &r));
return fails;
}

static int test_shaped_and_mirror(void) {
    int fails = 0;
    // an L-shape: asymmetric so mirroring is observable.
    block_id pat[] = { BLOCK_PLANKS, BLOCK_AIR,
                       BLOCK_PLANKS, BLOCK_PLANKS };
    craft_recipe r = craft_build_shaped("t_L", 2, 2, pat, BLOCK_FENCE, 1);

    craft_grid g;
    craft_grid_clear(&g);
    // place the exact L at an offset; translation invariance must hold.
    put(&g, 1, 0, BLOCK_PLANKS);
    put(&g, 1, 1, BLOCK_PLANKS);
    put(&g, 2, 1, BLOCK_PLANKS);
    CHECK(craft_match_shaped(&g, &r));
    CHECK(craft_match(&g, &r));

    // mirror the L horizontally; plain shaped fails, mirror match succeeds,
    // and craft_match accepts it because we allow mirrored shaped recipes.
    craft_grid_clear(&g);
    put(&g, 0, 0, BLOCK_AIR);   // explicit, no-op
    put(&g, 1, 0, BLOCK_PLANKS);
    put(&g, 0, 1, BLOCK_PLANKS);
    put(&g, 1, 1, BLOCK_PLANKS);
    CHECK(!craft_match_shaped(&g, &r));
    CHECK(craft_match_shaped_mirrored(&g, &r));
    CHECK(craft_match(&g, &r));
    return fails;
}

static int test_session_yield(void) {
    int fails = 0;
craft_session s;
craft_session_init(&s, 1);
CHECK(craft_session_place(&s, 0, 0, BLOCK_WOOD) == 1);
CHECK(craft_session_can_craft(&s));
craft_stack out;
CHECK(craft_session_craft_one(&s, &out));
CHECK(out.id == BLOCK_PLANKS && out.count == 4);
CHECK(!craft_session_can_craft(&s));
craft_session_init(&s, 1);
craft_grid_set(&s.grid, 0, 0, craft_stack_make(BLOCK_WOOD, 2));
craft_session_refresh(&s);
block_id rid;
int total = craft_session_craft_all(&s, &rid);
CHECK(rid == BLOCK_PLANKS);
CHECK(total == 8);
return fails;
}

static int test_region_gate(void) {
    int fails = 0;
    // a 2x2 inventory session must refuse a 3x3 chest recipe.
    craft_session s;
    craft_session_init(&s, 0);
    for (int y = 0; y < 3; y++)
        for (int x = 0; x < 3; x++)
            if (!(x == 1 && y == 1))
                craft_session_place(&s, x, y, BLOCK_PLANKS);
    CHECK(!craft_session_can_craft(&s));   // out of region, gated
    return fails;
}

static int test_query(void) {
    int fails = 0;
int ids[16];
int n = craft_query_by_result(BLOCK_PLANKS, ids, 16);
CHECK(n >= 1);
int planks_id = ids[0];
block_id pool_ok[] = { BLOCK_WOOD, BLOCK_STONE }
;
;
CHECK(craft_query_affordable(planks_id, pool_ok, 2));
CHECK(!craft_query_affordable(planks_id, pool_no, 1));
return fails;
