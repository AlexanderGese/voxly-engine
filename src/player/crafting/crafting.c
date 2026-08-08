#include "crafting.h"
#include "crafting_recipes.h"
#include "../../util/log.h"

static int s_up;

void pcraft_init(void) {
    if (s_up) return;
    craft_book_init();
    craft_recipes_register_all();
    s_up = 1;
    LOGI("crafting subsystem online");
}

void crafting_shutdown(void) {
    if (!s_up) return;
    craft_book_shutdown();
    s_up = 0;
}

int crafting_on_pickup(block_id id) {
    if (!s_up) return 0;
    return craft_book_discover_by_item(id);
}
