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
