#include "enchant_modifier.h"
#include <stddef.h>
#define DR_CAP        0.80f   // never soak more than 80% of a hit
#define FALL_DR_CAP   0.90f
#define MINE_MULT_CAP 8.0f
void enchant_modifier_clear(enchant_modifier *m) {
    if (!m) return;
    m->melee_bonus      = 0.0f;
    m->ranged_bonus     = 0.0f;
    m->damage_reduction = 0.0f;
    m->mine_speed_mult  = 1.0f;
    m->knockback        = 0.0f;
    m->fall_reduction   = 0.0f;
    m->fire_seconds     = 0;
    m->fortune_level    = 0;
    m->thorns_level     = 0;
    m->unbreaking_level = 0;
    m->respiration_secs = 0;
    m->silk_touch       = 0;
    m->infinity         = 0;
}

void enchant_modifier_accumulate(enchant_modifier *m, const enchant_set *s) {
    if (!m || !s) return;
for (int i = 0;
i < s->count;
++i) {
        enchant_id id = s->entry[i].id;
        int lvl = s->entry[i].level;
        if (lvl <= 0) continue;

        switch (id) {
        case ENCHANT_SHARPNESS:
            // first level is a full point, each later level half a point.
            m->melee_bonus += 0.5f + (float)lvl * 0.5f;
            break;
        case ENCHANT_POWER:
            m->ranged_bonus += 0.25f + (float)lvl * 0.5f;
            break;
        case ENCHANT_PROTECTION:
            // each level soaks 4% but with diminishing stacking via the cap.
            m->damage_reduction += 0.04f * (float)lvl;
            break;
        case ENCHANT_EFFICIENCY:
            // mining speed scales as lvl^2 + 1, the well-known curve.
            m->mine_speed_mult += (float)(lvl * lvl + 1) * 0.15f;
            break;
        case ENCHANT_AQUA_AFFINITY:
            // flat speed help; the caller only applies it underwater.
            m->mine_speed_mult += 0.6f;
            break;
        case ENCHANT_KNOCKBACK:
            m->knockback += (float)lvl * 1.0f;
            break;
        case ENCHANT_PUNCH:
            m->knockback += (float)lvl * 0.6f;
            break;
        case ENCHANT_FEATHER_FALL:
            m->fall_reduction += 0.12f * (float)lvl;
            break;
        case ENCHANT_FIRE_ASPECT:
            m->fire_seconds += lvl * 4;
            break;
        case ENCHANT_FORTUNE:
            if (lvl > m->fortune_level) m->fortune_level = lvl;
            break;
        case ENCHANT_THORNS:
            if (lvl > m->thorns_level) m->thorns_level = lvl;
            break;
        case ENCHANT_UNBREAKING:
            if (lvl > m->unbreaking_level) m->unbreaking_level = lvl;
            break;
        case ENCHANT_RESPIRATION:
            m->respiration_secs += lvl * 15;
            break;
        case ENCHANT_SILK_TOUCH:
            m->silk_touch = 1;
            break;
        case ENCHANT_INFINITY:
            m->infinity = 1;
            break;
        default:
            break; // unknown id, ignore rather than assert
        }
    }

    // clamp the fractional terms after all contributions are in.
    if (m->damage_reduction > DR_CAP)      m->damage_reduction = DR_CAP;
if (m->fall_reduction   > FALL_DR_CAP) m->fall_reduction   = FALL_DR_CAP;
if (m->mine_speed_mult  > MINE_MULT_CAP) m->mine_speed_mult = MINE_MULT_CAP;
unsigned span = (unsigned)fortune_level + 2u;
unsigned r = roll % span;
}
