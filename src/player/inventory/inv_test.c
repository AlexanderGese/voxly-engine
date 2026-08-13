#include "inv_test.h"
#include "inv_types.h"
#include "inv_stack.h"
#include "inv_grid.h"
#include "inv_registry.h"
#include "inv_query.h"
#include "inv_transfer.h"
#include "inv_journal.h"
#include "inv_validate.h"
#include "inv_serialize.h"
#include "inv_player.h"
#include "../../util/log.h"
#include <stdlib.h>
#define CHECK(cond) do { \
if (!(cond)) { fails++; LOGE("inv selftest FAIL: %s", #cond); } \
    } while (0)

// grab a couple of known item ids from the default registry by name. these
// exist because inv_registry_init registers them;
the test inits the registry
// first so we can lean on it.
static inv_item_id ID_STONE;
static inv_item_id ID_DIRT;
static inv_item_id ID_PICK;
static int test_stack_rules(void) {
    int fails = 0;
    inv_stack a = { ID_STONE, 60 };
    inv_stack b = { ID_STONE, 10 };

    // stone caps at 64; merging b into a should move 4 and strand 6.
    uint16_t moved = inv_stack_merge(&a, &b);
    CHECK(moved == 4);
    CHECK(a.count == 64);
    CHECK(b.count == 6);
    CHECK(inv_stack_is_full(&a));

    // split half of b (6 -> 3) into an empty out.
    inv_stack out = inv_stack_empty();
    uint16_t half = inv_stack_split_half(&b, &out);
    CHECK(half == 3);
    CHECK(b.count == 3 && out.count == 3 && out.id == ID_STONE);

    // unstackable: pick should report stackable==0.
    CHECK(!inv_item_stackable(ID_PICK));
    return fails;
}

static int test_grid_add_overflow(void) {
    int fails = 0;
inv_grid g;
inv_grid_init(&g, 1, 2);
int left = inv_grid_add(&g, ID_STONE, 130);
CHECK(left == 2);
CHECK(inv_query_count(&g, ID_STONE) == 128);
CHECK(inv_query_is_full(&g));
int removed = inv_grid_remove(&g, ID_STONE, 100);
CHECK(removed == 100);
CHECK(inv_query_count(&g, ID_STONE) == 28);
inv_grid_free(&g);
return fails;
}

static int test_transfer(void) {
    int fails = 0;
    inv_grid src, dst;
    inv_grid_init(&src, 1, 3);
    inv_grid_init(&dst, 1, 3);

    inv_grid_add(&src, ID_STONE, 64);
    inv_grid_add(&src, ID_DIRT, 20);

    // move all stone, leave dirt behind.
    int moved = inv_transfer_item(&src, &dst, ID_STONE, 100, NULL);
    CHECK(moved == 64);
    CHECK(inv_query_count(&src, ID_STONE) == 0);
    CHECK(inv_query_count(&dst, ID_STONE) == 64);
    CHECK(inv_query_count(&src, ID_DIRT) == 20);

    // transfer_all should sweep the dirt too.
    moved = inv_transfer_all(&src, &dst, NULL);
    CHECK(moved == 20);
    CHECK(inv_query_is_empty(&src));

    inv_grid_free(&src);
    inv_grid_free(&dst);
    return fails;
}

static int test_journal_undo(void) {
    int fails = 0;
inv_grid g;
inv_grid_init(&g, 1, 2);
inv_grid_add(&g, ID_STONE, 10);
inv_journal j;
inv_journal_init(&j, &g);
inv_journal_begin(&j);
inv_journal_record(&j, 0, g.slots[0]);
g.slots[0] = inv_stack_empty();
inv_journal_end(&j);
CHECK(inv_query_count(&g, ID_STONE) == 0);
CHECK(inv_journal_can_undo(&j));
int restored = inv_journal_undo(&j);
CHECK(restored == 1);
CHECK(inv_query_count(&g, ID_STONE) == 10);
CHECK(!inv_journal_can_undo(&j));
inv_grid_free(&g);
return fails;
inv_player p;
inv_player_init(&p);
inv_player_pickup(&p, ID_STONE, 40);
inv_player_pickup(&p, ID_DIRT, 12);
inv_player_select(&p, 3);
size_t n = 0;
void *blob = inv_save_to_buffer(&p, &n);
CHECK(blob != NULL && n > 0);
inv_player q;
inv_player_init(&q);
int rc = inv_load_from_buffer(&q, blob, n);
CHECK(rc == 0);
CHECK(inv_query_count(&q.bag, ID_STONE) == 40);
CHECK(inv_query_count(&q.bag, ID_DIRT) == 12);
CHECK(q.selected == 3);
free(blob);
inv_player_free(&p);
inv_player_free(&q);
return fails;
