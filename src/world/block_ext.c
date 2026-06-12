#include "block_ext.h"
#include "../util/log.h"

#include <string.h>

static block_ext_info g_ext[BLOCK_EXT_COUNT];
static int inited = 0;

static void reg(int id, const char *name, int solid, int opaque,
                int emits, int lum, int ttop, int tbot, int tside,
                int slab, int plant, int tool, float btime,
                int drop_id, int drop_count, int stack) {
    if (id < 0 || id >= BLOCK_EXT_COUNT) return;
    block_ext_info *b = &g_ext[id];
    b->name = name;
    b->solid = solid;
    b->opaque = opaque;
    b->emits_light = emits;
    b->luminance = lum;
    b->tile_top = ttop;
    b->tile_bot = tbot;
    b->tile_side = tside;
    b->is_slab = slab;
    b->is_plant = plant;
    b->tool_required = tool;
    b->break_time = btime;
    b->drop_id = drop_id;
    b->drop_count = drop_count;
    b->stack_max = stack;
}

void block_ext_init(void) {
    if (inited) return;
    memset(g_ext, 0, sizeof g_ext);

    // copy base blocks into the extended registry
    for (int i = 0; i < BLOCK_COUNT; i++) {
        const block_info *bi = block_get(i);
        reg(i, bi->name, bi->solid, bi->opaque, bi->emits_light, bi->luminance,
            bi->tile_top, bi->tile_bot, bi->tile_side,
            0, 0, 0, 1.0f, i, 1, 64);
    }

    // new blocks
    // id                  name           sol opq emt lum top bot sid slb plt tol  btime drop         cnt stk
    reg(BLOCK_IRON_ORE,    "iron_ore",     1, 1, 0, 0,  17, 17, 17, 0, 0, 1, 3.0f, BLOCK_IRON_ORE,    1, 64);
    reg(BLOCK_COAL_ORE,    "coal_ore",     1, 1, 0, 0,  18, 18, 18, 0, 0, 1, 2.5f, BLOCK_COAL_ORE,    1, 64);
    reg(BLOCK_GOLD_ORE,    "gold_ore",     1, 1, 0, 0,  19, 19, 19, 0, 0, 1, 4.0f, BLOCK_GOLD_ORE,    1, 64);
    reg(BLOCK_DIAMOND_ORE, "diamond_ore",  1, 1, 0, 0,  20, 20, 20, 0, 0, 1, 5.0f, BLOCK_DIAMOND_ORE, 1, 64);
    reg(BLOCK_GRAVEL,      "gravel",       1, 1, 0, 0,  21, 21, 21, 0, 0, 3, 0.8f, BLOCK_GRAVEL,      1, 64);
    reg(BLOCK_CLAY,        "clay",         1, 1, 0, 0,  22, 22, 22, 0, 0, 3, 0.8f, BLOCK_CLAY,        4, 64);
    reg(BLOCK_BOOKSHELF,   "bookshelf",    1, 1, 0, 0,  23, 8,  23, 0, 0, 2, 1.5f, BLOCK_BOOKSHELF,   1, 64);
    reg(BLOCK_MOSSY_COBBLE,"mossy_cobble", 1, 1, 0, 0,  24, 24, 24, 0, 0, 1, 2.0f, BLOCK_MOSSY_COBBLE,1, 64);
    reg(BLOCK_OBSIDIAN,    "obsidian",     1, 1, 0, 0,  25, 25, 25, 0, 0, 1, 10.0f,BLOCK_OBSIDIAN,    1, 64);
    reg(BLOCK_TNT,         "tnt",          1, 1, 0, 0,  26, 27, 28, 0, 0, 0, 0.0f, BLOCK_TNT,         1, 64);
    reg(BLOCK_WOOL_WHITE,  "wool_white",   1, 1, 0, 0,  29, 29, 29, 0, 0, 0, 0.8f, BLOCK_WOOL_WHITE,  1, 64);
    reg(BLOCK_WOOL_RED,    "wool_red",     1, 1, 0, 0,  30, 30, 30, 0, 0, 0, 0.8f, BLOCK_WOOL_RED,    1, 64);
    reg(BLOCK_WOOL_BLUE,   "wool_blue",    1, 1, 0, 0,  31, 31, 31, 0, 0, 0, 0.8f, BLOCK_WOOL_BLUE,   1, 64);
    reg(BLOCK_WOOL_GREEN,  "wool_green",   1, 1, 0, 0,  32, 32, 32, 0, 0, 0, 0.8f, BLOCK_WOOL_GREEN,  1, 64);
    reg(BLOCK_SLAB_STONE,  "slab_stone",   1, 0, 0, 0,   1,  1,  1, 1, 0, 1, 2.0f, BLOCK_SLAB_STONE,  1, 64);
    reg(BLOCK_SLAB_WOOD,   "slab_wood",    1, 0, 0, 0,   8,  8,  8, 1, 0, 2, 1.0f, BLOCK_SLAB_WOOD,   1, 64);
    reg(BLOCK_FENCE,       "fence",        1, 0, 0, 0,   8,  8,  8, 0, 0, 2, 1.0f, BLOCK_FENCE,       1, 64);
    reg(BLOCK_LADDER,      "ladder",       0, 0, 0, 0,  33, 33, 33, 0, 0, 2, 0.5f, BLOCK_LADDER,      1, 64);
    reg(BLOCK_DOOR_BOTTOM, "door_bottom",  1, 0, 0, 0,  34, 34, 34, 0, 0, 2, 1.0f, BLOCK_DOOR_BOTTOM, 1, 64);
    reg(BLOCK_DOOR_TOP,    "door_top",     1, 0, 0, 0,  35, 35, 35, 0, 0, 2, 1.0f, BLOCK_DOOR_BOTTOM, 0, 64);
    reg(BLOCK_CRAFTING_TABLE,"crafting_table",1,1,0,0,  36, 8,  37, 0, 0, 2, 1.5f, BLOCK_CRAFTING_TABLE,1,64);
    reg(BLOCK_FURNACE,     "furnace",      1, 1, 0, 0,  38, 1,  39, 0, 0, 1, 3.0f, BLOCK_FURNACE,     1, 64);
    reg(BLOCK_CHEST_BLOCK, "chest",        1, 0, 0, 0,  40, 40, 41, 0, 0, 2, 1.5f, BLOCK_CHEST_BLOCK, 1, 64);
    reg(BLOCK_FLOWER_RED,  "flower_red",   0, 0, 0, 0,  42, 42, 42, 0, 1, 0, 0.0f, BLOCK_FLOWER_RED,  1, 64);
    reg(BLOCK_FLOWER_YELLOW,"flower_yellow",0,0, 0, 0,  43, 43, 43, 0, 1, 0, 0.0f, BLOCK_FLOWER_YELLOW,1,64);
    reg(BLOCK_MUSHROOM_RED,"mushroom_red", 0, 0, 0, 0,  44, 44, 44, 0, 1, 0, 0.0f, BLOCK_MUSHROOM_RED,1, 64);
    reg(BLOCK_MUSHROOM_BROWN,"mush_brown", 0, 0, 0, 0,  45, 45, 45, 0, 1, 0, 0.0f, BLOCK_MUSHROOM_BROWN,1,64);
    reg(BLOCK_TALL_GRASS,  "tall_grass",   0, 0, 0, 0,  46, 46, 46, 0, 1, 0, 0.0f, BLOCK_AIR,         0, 64);
    reg(BLOCK_CACTUS,      "cactus",       1, 0, 0, 0,  47, 47, 48, 0, 0, 0, 0.5f, BLOCK_CACTUS,      1, 64);
    reg(BLOCK_SUGARCANE,   "sugarcane",    0, 0, 0, 0,  49, 49, 49, 0, 1, 0, 0.0f, BLOCK_SUGARCANE,   1, 64);
    reg(BLOCK_PUMPKIN,     "pumpkin",      1, 1, 0, 0,  50, 50, 51, 0, 0, 2, 1.0f, BLOCK_PUMPKIN,     1, 64);
    reg(BLOCK_MELON,       "melon",        1, 1, 0, 0,  52, 52, 53, 0, 0, 2, 1.0f, BLOCK_MELON,       1, 64);

    inited = 1;
    LOGI("block_ext: registered %d block types", BLOCK_EXT_COUNT);
}

const block_ext_info *block_ext_get(int id) {
    if (!inited) block_ext_init();
    if (id < 0 || id >= BLOCK_EXT_COUNT) return &g_ext[BLOCK_AIR];
    return &g_ext[id];
}

int block_ext_is_solid(int id)  { return block_ext_get(id)->solid; }
int block_ext_is_opaque(int id) { return block_ext_get(id)->opaque; }
int block_ext_is_plant(int id)  { return block_ext_get(id)->is_plant; }
float block_ext_break_time(int id) { return block_ext_get(id)->break_time; }
const char *block_ext_name(int id) { return block_ext_get(id)->name; }
