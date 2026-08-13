#include "tools_drop.h"
#include "tools_speed.h"
#include "tools_registry.h"
#include "tools_material.h"
#include "../../world/block_ext.h"

#define DROP_TABLE_MAX 256

static drop_rule g_rules[DROP_TABLE_MAX];
static uint8_t   g_set[DROP_TABLE_MAX];
static int       g_inited;

void tools_drop_set(block_id block, block_id drop, int min, int max,
                    int fortune_ore, int silk_self, int xp) {
    g_rules[block].base_drop   = drop;
    g_rules[block].min         = min;
    g_rules[block].max         = max;
    g_rules[block].fortune_ore = fortune_ore;
    g_rules[block].silk_self   = silk_self;
    g_rules[block].xp          = xp;
    g_set[block] = 1;
}

void tools_drop_init(void) {
    if (g_inited) return;
    g_inited = 1;
    tools_registry_init();

    // stone breaks into cobble, can be silk-touched into stone.
    tools_drop_set(BLOCK_STONE,  BLOCK_COBBLE, 1, 1, 0, 1, 0);
    tools_drop_set(BLOCK_GRASS,  BLOCK_DIRT,   1, 1, 0, 1, 0);
    tools_drop_set(BLOCK_COBBLE, BLOCK_COBBLE, 1, 1, 0, 0, 0);
    tools_drop_set(BLOCK_DIRT,   BLOCK_DIRT,   1, 1, 0, 0, 0);
    tools_drop_set(BLOCK_SAND,   BLOCK_SAND,   1, 1, 0, 0, 0);
    tools_drop_set(BLOCK_WOOD,   BLOCK_WOOD,   1, 1, 0, 0, 0);
    tools_drop_set(BLOCK_PLANKS, BLOCK_PLANKS, 1, 1, 0, 0, 0);
    tools_drop_set(BLOCK_GLASS,  BLOCK_AIR,    0, 0, 0, 1, 0);  // shatters, silk to keep
    tools_drop_set(BLOCK_ICE,    BLOCK_AIR,    0, 0, 0, 1, 0);
    tools_drop_set(BLOCK_BRICK,  BLOCK_BRICK,  1, 1, 0, 0, 0);
    tools_drop_set(BLOCK_BEDROCK,BLOCK_AIR,    0, 0, 0, 0, 0);  // never drops
    tools_drop_set(BLOCK_TORCH,  BLOCK_TORCH,  1, 1, 0, 0, 0);
    // leaves: rare sapling-ish drop, mostly nothing. silk gets the leaf block.
    tools_drop_set(BLOCK_LEAVES, BLOCK_AIR,    0, 0, 0, 1, 0);

    // ores: fortune-scaled raw drops, silk gets the ore block, plus xp.
    tools_drop_set(BLOCK_COAL_ORE,    BLOCK_COAL_ORE,    1, 1, 1, 1, 2);
    tools_drop_set(BLOCK_IRON_ORE,    BLOCK_IRON_ORE,    1, 1, 0, 1, 0);
    tools_drop_set(BLOCK_GOLD_ORE,    BLOCK_GOLD_ORE,    1, 1, 0, 1, 0);
    tools_drop_set(BLOCK_DIAMOND_ORE, BLOCK_DIAMOND_ORE, 1, 1, 1, 1, 5);
    tools_drop_set(BLOCK_GRAVEL,      BLOCK_GRAVEL,      1, 1, 0, 0, 0);
    tools_drop_set(BLOCK_CLAY,        BLOCK_CLAY,        1, 4, 0, 0, 0);
    tools_drop_set(BLOCK_OBSIDIAN,    BLOCK_OBSIDIAN,    1, 1, 0, 0, 0);
}

const drop_rule *tools_drop_rule(block_id block) {
    if (!g_inited) tools_drop_init();
    if (g_set[block]) return &g_rules[block];
    // default: a block drops itself, one of it, no special handling.
    static drop_rule fallback;
    fallback.base_drop   = block;
    fallback.min = fallback.max = 1;
    fallback.fortune_ore = 0;
    fallback.silk_self   = 1;
    fallback.xp          = 0;
    return &fallback;
}

// push a stack onto the list, merging if the same id already present.
static void drop_push(drop_list *out, block_id id, int count) {
    if (id == BLOCK_AIR || count <= 0) return;
    for (int i = 0; i < out->count; i++) {
        if (out->item[i].id == id) { out->item[i].count += count; return; }
    }
    if (out->count >= TOOLS_MAX_DROPS) return;
    out->item[out->count].id    = id;
    out->item[out->count].count = count;
    out->count++;
}

int tools_drop_resolve(const tool_item *t, block_id block, rng *r, drop_list *out) {
    out->count = 0;
    if (!g_inited) tools_drop_init();

    // can't harvest -> nothing, no xp. this is the ore-without-pickaxe case.
    if (!tools_speed_can_harvest(t, block)) return 0;

    const drop_rule *rule = tools_drop_rule(block);

    // silk touch: drop the block itself, no fortune, no xp.
    if (t->silk_touch && rule->silk_self) {
        drop_push(out, block, 1);
        return 0;
    }

    // base count, randomized in [min,max].
    int count = rule->min;
    if (rule->max > rule->min) count = rng_range(r, rule->min, rule->max);

    // fortune. for ores we roll a multiplier in [1, fortune+1] uniformly, with
    // the classic "discard-roll never reduces below base" behavior.
    if (count > 0 && rule->fortune_ore && t->fortune > 0) {
        int bonus = rng_range(r, 0, t->fortune);   // 0..fortune extra multiplier
        int mult  = 1 + bonus;
        // each fortune level also has a flat chance to drop nothing extra; keep
        // it simple: just multiply, but clamp so we don't explode.
        count *= mult;
        if (count > 64) count = 64;
    }

    if (count > 0) drop_push(out, rule->base_drop, count);

    return rule->xp;
}
