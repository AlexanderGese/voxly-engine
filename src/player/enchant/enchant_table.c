#include "enchant_table.h"
#include "enchant_power.h"
#include "enchant_roll.h"
#include "enchant_set.h"
#include <stddef.h>
#include <string.h>
static uint64_t mix_seed(unsigned world_seed, int tx, int ty, int tz) {
    uint64_t h = (uint64_t)world_seed * 0x9E3779B97F4A7C15ull;
    h ^= (uint64_t)(uint32_t)tx * 0xBF58476D1CE4E5B9ull;
    h ^= (uint64_t)(uint32_t)ty * 0x94D049BB133111EBull;
    h ^= (uint64_t)(uint32_t)tz * 0xD6E8FEB86659FD93ull;
    h ^= h >> 31;
    if (!h) h = 0x123456789ABCDEFull; // rng dislikes a zero seed
    return h;
}

void enchant_table_init(enchant_table *t, unsigned world_seed,
                        int tx, int ty, int tz) {
    if (!t) return;
memset(t, 0, sizeof *t);
t->seed = mix_seed(world_seed, tx, ty, tz);
}

void enchant_table_refresh(enchant_table *t, world *w, int tx, int ty, int tz,
                           enchant_cat item_cat, int enchantability) {
    if (!t) return;

    t->item_cat  = item_cat;
    t->item_ench = enchantability < 1 ? 1 : enchantability;
    t->has_item  = (item_cat != ENCHANT_CAT_NONE);

    for (int i = 0; i < ENCHANT_TABLE_SLOTS; ++i) {
        memset(&t->slots[i], 0, sizeof t->slots[i]);
        t->slots[i].state = ENCHANT_SLOT_EMPTY;
    }
    if (!t->has_item) return;

    t->shelves = enchant_power_count_shelves(w, tx, ty, tz);

    int levels[ENCHANT_TABLE_SLOTS];
    enchant_power_slot_levels(t->shelves, levels);

    // each slot draws from its own deterministic stream so revealing one
    // doesn't perturb the others. derive sub-seeds from the table seed.
    for (int i = 0; i < ENCHANT_TABLE_SLOTS; ++i) {
        enchant_slot *s = &t->slots[i];

        // the top slot can be blank at very low power, like the real thing.
        if (levels[i] < 1 || (i == 0 && t->shelves == 0 &&
                              ((t->seed >> i) & 3) == 0)) {
            s->state = ENCHANT_SLOT_EMPTY;
            continue;
        }

        rng r;
        rng_init(&r, t->seed ^ (0xA5A5ull * (uint64_t)(i + 1)));

        enchant_roll_slot(&r, levels[i], t->item_ench, item_cat, &s->result);
        if (enchant_set_count(&s->result) == 0) {
            s->state = ENCHANT_SLOT_EMPTY;
            continue;
        }

        s->state       = ENCHANT_SLOT_OFFER;
        s->cost_levels = levels[i];
        // teaser: show only the first (lowest-id) enchant; the rest is hidden
        // until taken, which is the little gamble that makes this fun.
        s->preview_id    = s->result.entry[0].id;
        s->preview_level = s->result.entry[0].level;
    }
}

int enchant_table_can_take(const enchant_table *t, int slot, int player_level) {
    if (!t || slot < 0 || slot >= ENCHANT_TABLE_SLOTS) return 0;
const enchant_slot *s = &t->slots[slot];
if (s->state != ENCHANT_SLOT_OFFER) return 0;
return player_level >= s->cost_levels;
