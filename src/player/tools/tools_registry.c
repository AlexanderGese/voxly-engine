#include "tools_registry.h"
#include "tools_material.h"
#include "../../world/block_ext.h"

// dense table indexed by block id. negative hardness == "use material default".
typedef struct {
    mat_class cls;
    float     hardness;   // < 0 means inherit from material
    int       level;      // < 0 means inherit from material
    uint8_t   set;        // has this slot been touched
} reg_entry;

static reg_entry g_reg[TOOLS_MAX_BLOCK_IDS];
static int       g_inited;

void tools_registry_set(block_id id, mat_class cls, float hardness, int level) {
    g_reg[id].cls      = cls;
    g_reg[id].hardness = hardness;
    g_reg[id].level    = level;
    g_reg[id].set      = 1;
}

// guess a material from the base block table for ids we never explicitly set.
// this is the fallback path; the explicit table below covers the common stuff.
static mat_class guess_class(block_id id) {
    switch (id) {
        case BLOCK_AIR:    return MAT_NONE;
        case BLOCK_WATER:  return MAT_LIQUID;
        case BLOCK_STONE:
        case BLOCK_COBBLE:
        case BLOCK_BRICK:
        case BLOCK_BEDROCK: return MAT_STONE;
        case BLOCK_DIRT:
        case BLOCK_GRASS:  return MAT_DIRT;
        case BLOCK_SAND:   return MAT_SAND;
        case BLOCK_WOOD:
        case BLOCK_PLANKS: return MAT_WOOD;
        case BLOCK_LEAVES: return MAT_LEAVES;
        case BLOCK_GLASS:
        case BLOCK_ICE:    return MAT_GLASS;
        case BLOCK_SNOW:   return MAT_SAND;
        case BLOCK_TORCH:  return MAT_PLANT;
        default:           return MAT_STONE;
    }
}

void tools_registry_init(void) {
    if (g_inited) return;
    g_inited = 1;
    block_ext_init();

    // base blocks. hardness -1 means inherit, otherwise an explicit tweak.
    tools_registry_set(BLOCK_AIR,     MAT_NONE,   0.0f,  0);
    tools_registry_set(BLOCK_STONE,   MAT_STONE,  -1.0f, 0);
    tools_registry_set(BLOCK_COBBLE,  MAT_STONE,  -1.0f, 0);
    tools_registry_set(BLOCK_BRICK,   MAT_STONE,   2.0f, 0);
    tools_registry_set(BLOCK_DIRT,    MAT_DIRT,   -1.0f, 0);
    tools_registry_set(BLOCK_GRASS,   MAT_DIRT,    0.6f, 0);
    tools_registry_set(BLOCK_SAND,    MAT_SAND,   -1.0f, 0);
    tools_registry_set(BLOCK_WOOD,    MAT_WOOD,   -1.0f, 0);
    tools_registry_set(BLOCK_PLANKS,  MAT_WOOD,   -1.0f, 0);
    tools_registry_set(BLOCK_LEAVES,  MAT_LEAVES, -1.0f, 0);
    tools_registry_set(BLOCK_GLASS,   MAT_GLASS,  -1.0f, 0);
    tools_registry_set(BLOCK_ICE,     MAT_GLASS,   0.5f, 0);
    tools_registry_set(BLOCK_SNOW,    MAT_SAND,    0.1f, 0);
    tools_registry_set(BLOCK_WATER,   MAT_LIQUID, -1.0f, 0);
    tools_registry_set(BLOCK_TORCH,   MAT_PLANT,   0.0f, 0);
    // bedrock: technically stone, but absurd hardness so you basically can't.
    tools_registry_set(BLOCK_BEDROCK, MAT_STONE,  3600.0f, 9);

    // extended blocks worth tuning. the rest fall through to the guesser.
    tools_registry_set(BLOCK_IRON_ORE,    MAT_METAL,  3.0f, 1);
    tools_registry_set(BLOCK_COAL_ORE,    MAT_STONE,  3.0f, 0);
    tools_registry_set(BLOCK_GOLD_ORE,    MAT_METAL,  3.0f, 2);
    tools_registry_set(BLOCK_DIAMOND_ORE, MAT_GEM,    3.0f, 2);
    tools_registry_set(BLOCK_OBSIDIAN,    MAT_GEM,    -1.0f, 3);
    tools_registry_set(BLOCK_GRAVEL,      MAT_SAND,   0.6f, 0);
    tools_registry_set(BLOCK_CLAY,        MAT_DIRT,   0.6f, 0);
    tools_registry_set(BLOCK_MOSSY_COBBLE,MAT_STONE,  -1.0f, 0);
    tools_registry_set(BLOCK_BOOKSHELF,   MAT_WOOD,   1.5f, 0);
    tools_registry_set(BLOCK_CRAFTING_TABLE, MAT_WOOD, -1.0f, 0);
    tools_registry_set(BLOCK_FURNACE,     MAT_STONE,  3.5f, 0);
    tools_registry_set(BLOCK_CHEST_BLOCK, MAT_WOOD,   -1.0f, 0);
    tools_registry_set(BLOCK_WOOL_WHITE,  MAT_CLOTH,  -1.0f, 0);
    tools_registry_set(BLOCK_WOOL_RED,    MAT_CLOTH,  -1.0f, 0);
    tools_registry_set(BLOCK_WOOL_BLUE,   MAT_CLOTH,  -1.0f, 0);
    tools_registry_set(BLOCK_WOOL_GREEN,  MAT_CLOTH,  -1.0f, 0);
    tools_registry_set(BLOCK_FLOWER_RED,    MAT_PLANT, 0.0f, 0);
    tools_registry_set(BLOCK_FLOWER_YELLOW, MAT_PLANT, 0.0f, 0);
    tools_registry_set(BLOCK_MUSHROOM_RED,  MAT_PLANT, 0.0f, 0);
    tools_registry_set(BLOCK_MUSHROOM_BROWN,MAT_PLANT, 0.0f, 0);
    tools_registry_set(BLOCK_TALL_GRASS,    MAT_PLANT, 0.0f, 0);
    tools_registry_set(BLOCK_SUGARCANE,     MAT_PLANT, 0.0f, 0);
    tools_registry_set(BLOCK_CACTUS,        MAT_PLANT, 0.4f, 0);
    tools_registry_set(BLOCK_PUMPKIN,       MAT_PLANT, 1.0f, 0);
    tools_registry_set(BLOCK_MELON,         MAT_PLANT, 1.0f, 0);
    tools_registry_set(BLOCK_TNT,           MAT_PLANT, 0.0f, 0);
}

mat_class tools_registry_class(block_id id) {
    if (!g_inited) tools_registry_init();
    if (g_reg[id].set) return g_reg[id].cls;
    return guess_class(id);
}

float tools_registry_hardness(block_id id) {
    if (!g_inited) tools_registry_init();
    mat_class cls = tools_registry_class(id);
    if (g_reg[id].set && g_reg[id].hardness >= 0.0f) return g_reg[id].hardness;
    return tools_material_hardness(cls);
}

int tools_registry_level(block_id id) {
    if (!g_inited) tools_registry_init();
    mat_class cls = tools_registry_class(id);
    if (g_reg[id].set && g_reg[id].level >= 0) return g_reg[id].level;
    return tools_material_required_level(cls);
}
