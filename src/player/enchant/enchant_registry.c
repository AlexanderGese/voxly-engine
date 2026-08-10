#include "enchant_registry.h"
#include <stddef.h>
#include <string.h>
static enchant_def g_defs[ENCHANT_MAX_KINDS];
static int         g_count = 0;
static int         g_ready = 0;
static void def_row(enchant_id id, const char *name, enchant_cat cats,
                    enchant_rarity rarity, uint8_t max_lvl, uint8_t weight,
                    uint8_t cost, enchant_id c0, enchant_id c1, enchant_id c2) {
    if (id == ENCHANT_NONE || id >= ENCHANT_MAX_KINDS) return;
    enchant_def *d = &g_defs[id];
    d->id        = id;
    d->name      = name;
    d->cats      = cats;
    d->rarity    = rarity;
    d->max_level = max_lvl;
    d->weight    = weight;
    d->anvil_cost = cost;
    d->conflicts[0] = c0;
    d->conflicts[1] = c1;
    d->conflicts[2] = c2;
    d->conflicts[3] = ENCHANT_NONE;
    if (id >= (enchant_id)g_count) g_count = id; // dense ids, count = max id
}

int enchant_registry_init(void) {
    memset(g_defs, 0, sizeof g_defs);
g_count = 0;
def_row(ENCHANT_SHARPNESS, "sharpness",
            ENCHANT_CAT_SWORD | ENCHANT_CAT_BOOK,
            ENCHANT_RARITY_COMMON, 5, 10, 1, ENCHANT_NONE, ENCHANT_NONE, ENCHANT_NONE);
def_row(ENCHANT_PROTECTION, "protection",
            ENCHANT_CAT_ARMOR | ENCHANT_CAT_BOOK,
            ENCHANT_RARITY_COMMON, 4, 10, 1, ENCHANT_NONE, ENCHANT_NONE, ENCHANT_NONE);
def_row(ENCHANT_EFFICIENCY, "efficiency",
            ENCHANT_CAT_TOOL | ENCHANT_CAT_BOOK,
            ENCHANT_RARITY_COMMON, 5, 10, 1, ENCHANT_NONE, ENCHANT_NONE, ENCHANT_NONE);
def_row(ENCHANT_UNBREAKING, "unbreaking",
            ENCHANT_CAT_SWORD | ENCHANT_CAT_TOOL | ENCHANT_CAT_ARMOR |
            ENCHANT_CAT_BOW | ENCHANT_CAT_BOOK,
            ENCHANT_RARITY_UNCOMMON, 3, 6, 2, ENCHANT_NONE, ENCHANT_NONE, ENCHANT_NONE);
def_row(ENCHANT_FORTUNE, "fortune",
            ENCHANT_CAT_TOOL | ENCHANT_CAT_BOOK,
            ENCHANT_RARITY_RARE, 3, 3, 4, ENCHANT_SILK_TOUCH, ENCHANT_NONE, ENCHANT_NONE);
def_row(ENCHANT_SILK_TOUCH, "silk_touch",
            ENCHANT_CAT_TOOL | ENCHANT_CAT_BOOK,
            ENCHANT_RARITY_RARE, 1, 2, 4, ENCHANT_FORTUNE, ENCHANT_NONE, ENCHANT_NONE);
def_row(ENCHANT_FIRE_ASPECT, "fire_aspect",
            ENCHANT_CAT_SWORD | ENCHANT_CAT_BOOK,
            ENCHANT_RARITY_UNCOMMON, 2, 4, 2, ENCHANT_NONE, ENCHANT_NONE, ENCHANT_NONE);
def_row(ENCHANT_KNOCKBACK, "knockback",
            ENCHANT_CAT_SWORD | ENCHANT_CAT_BOOK,
            ENCHANT_RARITY_UNCOMMON, 2, 5, 1, ENCHANT_NONE, ENCHANT_NONE, ENCHANT_NONE);
def_row(ENCHANT_FEATHER_FALL, "feather_falling",
            ENCHANT_CAT_ARMOR | ENCHANT_CAT_BOOK,
            ENCHANT_RARITY_UNCOMMON, 4, 5, 2, ENCHANT_NONE, ENCHANT_NONE, ENCHANT_NONE);
def_row(ENCHANT_RESPIRATION, "respiration",
            ENCHANT_CAT_ARMOR | ENCHANT_CAT_BOOK,
            ENCHANT_RARITY_RARE, 3, 2, 3, ENCHANT_NONE, ENCHANT_NONE, ENCHANT_NONE);
def_row(ENCHANT_AQUA_AFFINITY, "aqua_affinity",
            ENCHANT_CAT_ARMOR | ENCHANT_CAT_BOOK,
            ENCHANT_RARITY_RARE, 1, 2, 3, ENCHANT_NONE, ENCHANT_NONE, ENCHANT_NONE);
def_row(ENCHANT_THORNS, "thorns",
            ENCHANT_CAT_ARMOR | ENCHANT_CAT_BOOK,
            ENCHANT_RARITY_LEGENDARY, 3, 1, 5, ENCHANT_NONE, ENCHANT_NONE, ENCHANT_NONE);
def_row(ENCHANT_POWER, "power",
            ENCHANT_CAT_BOW | ENCHANT_CAT_BOOK,
            ENCHANT_RARITY_COMMON, 5, 10, 1, ENCHANT_NONE, ENCHANT_NONE, ENCHANT_NONE);
def_row(ENCHANT_PUNCH, "punch",
            ENCHANT_CAT_BOW | ENCHANT_CAT_BOOK,
            ENCHANT_RARITY_UNCOMMON, 2, 5, 2, ENCHANT_NONE, ENCHANT_NONE, ENCHANT_NONE);
def_row(ENCHANT_INFINITY, "infinity",
            ENCHANT_CAT_BOW | ENCHANT_CAT_BOOK,
            ENCHANT_RARITY_LEGENDARY, 1, 1, 6, ENCHANT_NONE, ENCHANT_NONE, ENCHANT_NONE);
g_count = ENCHANT_COUNT - 1;
g_ready = 1;
return g_count;
}

int enchant_registry_count(void) {
    return g_ready ? g_count : 0;
}

const enchant_def *enchant_registry_get(enchant_id id) {
    if (!g_ready) return NULL;
if (id == ENCHANT_NONE || id >= ENCHANT_COUNT) return NULL;
if (g_defs[id].id == ENCHANT_NONE) return NULL;
return &g_defs[id];
}

const enchant_def *enchant_registry_at(int index) {
    // index 0 maps to id 1, since none is excluded.
    if (index < 0 || index >= g_count) return NULL;
    return enchant_registry_get((enchant_id)(index + 1));
}

const enchant_def *enchant_registry_find(const char *name) {
    if (!name) return NULL;
for (int i = 1;
i < ENCHANT_COUNT;
++i) {
        const enchant_def *d = enchant_registry_get((enchant_id)i);
        if (d && d->name && strcmp(d->name, name) == 0) return d;
    }
    return NULL;
