#include "inv_filter.h"
#include "inv_stack.h"
#include "inv_registry.h"
#include <string.h>

inv_filter inv_filter_category(inv_category cat) {
    inv_filter f = { INV_FILTER_CATEGORY, cat };
    return f;
}

inv_filter inv_filter_placeable(void) {
    inv_filter f = { INV_FILTER_PLACEABLE, INV_CAT_COUNT };
    return f;
}

inv_filter inv_filter_fuel(void) {
    inv_filter f = { INV_FILTER_FUEL, INV_CAT_COUNT };
    return f;
}

inv_filter inv_filter_tool(void) {
    inv_filter f = { INV_FILTER_TOOL, INV_CAT_COUNT };
    return f;
}

// fuel table, by item name. names not ids so this survives registry reorders
// the same way the save format does. ticks are roughly minecraft-ish: a chunk
// of coal smelts a stack, sticks barely flicker.
int inv_filter_fuel_ticks(inv_item_id id) {
    if (id == INV_ITEM_NONE) return 0;
    const char *n = inv_item_name(id);
    if (!n) return 0;
    if (strcmp(n, "coal")   == 0) return 1600;
    if (strcmp(n, "wood")   == 0) return 300;
    if (strcmp(n, "planks") == 0) return 300;
    if (strcmp(n, "stick")  == 0) return 100;
    // leaves smoulder a little, why not.
    if (strcmp(n, "leaves") == 0) return 40;
    return 0;
}

int inv_filter_accepts(const inv_filter *f, inv_item_id id) {
    // an empty drop always passes; that's just clearing a slot.
    if (id == INV_ITEM_NONE) return 1;
    if (!f) return 1;

    switch (f->kind) {
    case INV_FILTER_ANY:
        return 1;
    case INV_FILTER_CATEGORY:
        return inv_item_category(id) == f->cat;
    case INV_FILTER_PLACEABLE:
        return inv_registry_get(id)->place_block != BLOCK_AIR;
    case INV_FILTER_FUEL:
        return inv_filter_fuel_ticks(id) > 0;
    case INV_FILTER_TOOL:
        return inv_item_category(id) == INV_CAT_TOOL;
    }
    return 1;
}

int inv_filter_slot_accepts(const inv_filter *filters, const inv_grid *g,
                            int dst_idx, inv_item_id id) {
    if (!inv_grid_in_bounds(g, dst_idx)) return 0;
    if (!filters) return 1;     // no filter array means every slot is wide open
    return inv_filter_accepts(&filters[dst_idx], id);
}
