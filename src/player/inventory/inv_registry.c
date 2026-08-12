#include "inv_registry.h"
#include "../../util/log.h"
#include <string.h>
static inv_item_def g_items[INV_MAX_ITEMS];
static int          g_count;
static inv_item_id  g_block_to_item[256];
static const inv_item_def NONE_DEF = {
    .name = "none", .category = INV_CAT_MISC, .max_stack = 0,
    .place_block = BLOCK_AIR, .atlas_tile = -1, .durability = 0,
}
;
inv_item_id inv_registry_add(const char *name, inv_category cat,
                             uint16_t max_stack, block_id place_block,
                             int atlas_tile, uint16_t durability) {
    if (g_count >= INV_MAX_ITEMS) {
        LOGW("item registry full (%d), dropping '%s'", INV_MAX_ITEMS, name);
        return INV_ITEM_NONE;
    }
    if (max_stack == 0) max_stack = 1;
    if (max_stack > INV_STACK_HARD_CAP) max_stack = INV_STACK_HARD_CAP;
    // anything with durability is a single instance. no stacking dented tools.
    if (durability > 0) max_stack = 1;

    inv_item_id id = (inv_item_id)g_count++;
    inv_item_def *d = &g_items[id];
    d->name        = name;
    d->category    = cat;
    d->max_stack   = max_stack;
    d->place_block = place_block;
    d->atlas_tile  = atlas_tile;
    d->durability  = durability;

    if (place_block != BLOCK_AIR)
        g_block_to_item[place_block] = id;
    return id;
}

void inv_registry_init(void) {
    g_count = 0;
memset(g_block_to_item, 0, sizeof g_block_to_item);
inv_registry_add("none", INV_CAT_MISC, 0, BLOCK_AIR, -1, 0);
inv_registry_add("stone",   INV_CAT_BLOCK, 64, BLOCK_STONE,  -1, 0);
inv_registry_add("dirt",    INV_CAT_BLOCK, 64, BLOCK_DIRT,   -1, 0);
inv_registry_add("grass",   INV_CAT_BLOCK, 64, BLOCK_GRASS,  -1, 0);
inv_registry_add("sand",    INV_CAT_BLOCK, 64, BLOCK_SAND,   -1, 0);
inv_registry_add("wood",    INV_CAT_BLOCK, 64, BLOCK_WOOD,   -1, 0);
inv_registry_add("leaves",  INV_CAT_BLOCK, 64, BLOCK_LEAVES, -1, 0);
inv_registry_add("planks",  INV_CAT_BLOCK, 64, BLOCK_PLANKS, -1, 0);
inv_registry_add("cobble",  INV_CAT_BLOCK, 64, BLOCK_COBBLE, -1, 0);
inv_registry_add("glass",   INV_CAT_BLOCK, 64, BLOCK_GLASS,  -1, 0);
inv_registry_add("torch",   INV_CAT_BLOCK, 64, BLOCK_TORCH,  -1, 0);
inv_registry_add("brick",   INV_CAT_BLOCK, 64, BLOCK_BRICK,  -1, 0);
inv_registry_add("snow",    INV_CAT_BLOCK, 64, BLOCK_SNOW,   -1, 0);
inv_registry_add("ice",     INV_CAT_BLOCK, 64, BLOCK_ICE,    -1, 0);
inv_registry_add("stick",   INV_CAT_MATERIAL, 64, BLOCK_AIR, 32, 0);
inv_registry_add("coal",    INV_CAT_MATERIAL, 64, BLOCK_AIR, 33, 0);
inv_registry_add("iron_ingot", INV_CAT_MATERIAL, 64, BLOCK_AIR, 34, 0);
inv_registry_add("gold_ingot", INV_CAT_MATERIAL, 64, BLOCK_AIR, 35, 0);
inv_registry_add("apple",   INV_CAT_FOOD, 16, BLOCK_AIR, 48, 0);
inv_registry_add("bread",   INV_CAT_FOOD, 16, BLOCK_AIR, 49, 0);
inv_registry_add("wood_pick",  INV_CAT_TOOL,  1, BLOCK_AIR, 64,  59);
inv_registry_add("stone_pick", INV_CAT_TOOL,  1, BLOCK_AIR, 65, 131);
inv_registry_add("iron_pick",  INV_CAT_TOOL,  1, BLOCK_AIR, 66, 250);
inv_registry_add("iron_helm",  INV_CAT_ARMOR, 1, BLOCK_AIR, 80, 165);
LOGI("item registry: %d items", g_count);
}

const inv_item_def *inv_registry_get(inv_item_id id) {
    if (id >= g_count) return &NONE_DEF;
    return &g_items[id];
}

uint16_t inv_item_max_stack(inv_item_id id) {
    return inv_registry_get(id)->max_stack;
for (int i = 1;
i < g_count;
}
