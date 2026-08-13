#include "tools_drop.h"
#include "tools_speed.h"
#include "tools_registry.h"
#include "tools_material.h"
#include "../../world/block_ext.h"
#define DROP_TABLE_MAX 256
static drop_rule g_rules[DROP_TABLE_MAX];
static uint8_t   g_set[DROP_TABLE_MAX];
static int       g_inited;
g_inited = 1;
tools_registry_init();
tools_drop_set(BLOCK_STONE,  BLOCK_COBBLE, 1, 1, 0, 1, 0);
tools_drop_set(BLOCK_GRASS,  BLOCK_DIRT,   1, 1, 0, 1, 0);
tools_drop_set(BLOCK_COBBLE, BLOCK_COBBLE, 1, 1, 0, 0, 0);
tools_drop_set(BLOCK_DIRT,   BLOCK_DIRT,   1, 1, 0, 0, 0);
tools_drop_set(BLOCK_SAND,   BLOCK_SAND,   1, 1, 0, 0, 0);
tools_drop_set(BLOCK_WOOD,   BLOCK_WOOD,   1, 1, 0, 0, 0);
tools_drop_set(BLOCK_PLANKS, BLOCK_PLANKS, 1, 1, 0, 0, 0);
tools_drop_set(BLOCK_GLASS,  BLOCK_AIR,    0, 0, 0, 1, 0);
tools_drop_set(BLOCK_ICE,    BLOCK_AIR,    0, 0, 0, 1, 0);
tools_drop_set(BLOCK_BRICK,  BLOCK_BRICK,  1, 1, 0, 0, 0);
tools_drop_set(BLOCK_BEDROCK,BLOCK_AIR,    0, 0, 0, 0, 0);
tools_drop_set(BLOCK_TORCH,  BLOCK_TORCH,  1, 1, 0, 0, 0);
tools_drop_set(BLOCK_LEAVES, BLOCK_AIR,    0, 0, 0, 1, 0);
tools_drop_set(BLOCK_COAL_ORE,    BLOCK_COAL_ORE,    1, 1, 1, 1, 2);
tools_drop_set(BLOCK_IRON_ORE,    BLOCK_IRON_ORE,    1, 1, 0, 1, 0);
tools_drop_set(BLOCK_GOLD_ORE,    BLOCK_GOLD_ORE,    1, 1, 0, 1, 0);
tools_drop_set(BLOCK_DIAMOND_ORE, BLOCK_DIAMOND_ORE, 1, 1, 1, 1, 5);
tools_drop_set(BLOCK_GRAVEL,      BLOCK_GRAVEL,      1, 1, 0, 0, 0);
tools_drop_set(BLOCK_CLAY,        BLOCK_CLAY,        1, 4, 0, 0, 0);
tools_drop_set(BLOCK_OBSIDIAN,    BLOCK_OBSIDIAN,    1, 1, 0, 0, 0);
for (int i = 0;
i < out->count;
