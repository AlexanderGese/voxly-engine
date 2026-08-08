#include "crafting_book.h"
#include "crafting_match.h"
#include "../../util/log.h"
#include <string.h>

// fixed-cap store. we have maybe a few dozen recipes, no reason to grow a
// darray and chase pointers. the unlock bitset rides alongside.

#define CRAFT_BOOK_CAP 256

static craft_recipe s_recipes[CRAFT_BOOK_CAP];
static int          s_count;
static uint32_t     s_unlocked[(CRAFT_BOOK_CAP + 31) / 32];
static int          s_inited;

void craft_book_init(void) {
    if (s_inited) return;
    s_count = 0;
    memset(s_unlocked, 0, sizeof s_unlocked);
    s_inited = 1;
}

void craft_book_shutdown(void) {
    s_count = 0;
    s_inited = 0;
    memset(s_unlocked, 0, sizeof s_unlocked);
}

int craft_book_add(const craft_recipe *r) {
    if (s_count >= CRAFT_BOOK_CAP) {
        LOGW("recipe book full, dropping recipe '%s'",
             r->name ? r->name : "?");
        return -1;
    }
    int id = s_count++;
    s_recipes[id] = *r;
    s_recipes[id].id = id;
    return id;
}

int craft_book_count(void) { return s_count; }

const craft_recipe *craft_book_get(int id) {
    if (id < 0 || id >= s_count) return NULL;
    return &s_recipes[id];
}

int craft_book_find(const craft_grid *g) {
    for (int i = 0; i < s_count; i++)
        if (craft_match(g, &s_recipes[i])) return i;
    return -1;
}

int craft_book_find_unlocked(const craft_grid *g) {
    for (int i = 0; i < s_count; i++) {
        if (!craft_book_is_unlocked(i)) continue;
        if (craft_match(g, &s_recipes[i])) return i;
    }
    return -1;
}

// ---- unlock bitset ----

void craft_book_unlock(int recipe_id) {
    if (recipe_id < 0 || recipe_id >= s_count) return;
    s_unlocked[recipe_id >> 5] |= (1u << (recipe_id & 31));
}

int craft_book_is_unlocked(int recipe_id) {
    if (recipe_id < 0 || recipe_id >= s_count) return 0;
    return (s_unlocked[recipe_id >> 5] >> (recipe_id & 31)) & 1u;
}

int craft_book_unlocked_count(void) {
    int n = 0;
    for (int i = 0; i < s_count; i++)
        if (craft_book_is_unlocked(i)) n++;
    return n;
}

int craft_book_discover_by_item(block_id id) {
    int unlocked = 0;
    for (int i = 0; i < s_count; i++) {
        if (craft_book_is_unlocked(i)) continue;
        const craft_recipe *r = &s_recipes[i];
        for (int j = 0; j < r->ing_n; j++) {
            if (r->ing[j] == id) {
                craft_book_unlock(i);
                unlocked++;
                break;
            }
        }
    }
    if (unlocked)
        LOGD("crafting: discovered %d recipe(s) via item %d", unlocked, id);
    return unlocked;
}
