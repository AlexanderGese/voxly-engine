#include "combat_resist.h"
#include "combat_config.h"
#include "combat_damagetype.h"

// the half / double constants for grant/weakness. nothing magic, just felt
// right after a few playtests.
#define RESIST_GRANT_MULT    0.5f
#define RESIST_WEAK_MULT     1.5f

static const int k_material_points[COMBAT_ARMOR_MATERIAL_COUNT] = {
    [COMBAT_ARMOR_NONE]    = 0,
    [COMBAT_ARMOR_LEATHER] = 7,
    [COMBAT_ARMOR_CHAIN]   = 12,
    [COMBAT_ARMOR_IRON]    = 15,
    [COMBAT_ARMOR_GOLD]    = 11,
    [COMBAT_ARMOR_DIAMOND] = 20,
};

int combat_resist_material_points(combat_armor_material m) {
    if (m < 0 || m >= COMBAT_ARMOR_MATERIAL_COUNT) return 0;
    return k_material_points[m];
}

void combat_resist_clear(combat_combatant *c) {
    for (int i = 0; i < COMBAT_DMG_COUNT; i++) c->resist[i] = 1.0f;
}

void combat_resist_equip(combat_combatant *c, combat_armor_material m) {
    int pts = combat_resist_material_points(m);
    if (pts < 0) pts = 0;
    if (pts > 20) pts = 20;   // same cap combat_health_init enforces
    c->armor = pts;
}

void combat_resist_add(combat_combatant *c, combat_damage_type t, float mult) {
    if (t < 0 || t >= COMBAT_DMG_COUNT) return;
    if (mult < 0.0f) mult = 0.0f;
    c->resist[t] *= mult;
    // dont let it creep absurdly high from repeated weaknesses.
    if (c->resist[t] > 8.0f) c->resist[t] = 8.0f;
}

void combat_resist_grant(combat_combatant *c, combat_damage_type t) {
    combat_resist_add(c, t, RESIST_GRANT_MULT);
}

void combat_resist_weakness(combat_combatant *c, combat_damage_type t) {
    combat_resist_add(c, t, RESIST_WEAK_MULT);
}

void combat_resist_immune(combat_combatant *c, combat_damage_type t) {
    if (t < 0 || t >= COMBAT_DMG_COUNT) return;
    c->resist[t] = 0.0f;
}

float combat_resist_factor(const combat_combatant *c, combat_damage_type t) {
    if (t < 0 || t >= COMBAT_DMG_COUNT) return 1.0f;

    float f = c->resist[t];

    // fold in the flat armor the same way the mitigator does, so the preview
    // matches what an actual hit would do.
    if (!combat_dmg_ignores_armor(t) && c->armor > 0) {
        float reduce = (float)c->armor * COMBAT_ARMOR_PER_POINT;
        if (reduce > COMBAT_ARMOR_MAX_REDUCE) reduce = COMBAT_ARMOR_MAX_REDUCE;
        f *= (1.0f - reduce);
    }

    if (f < 0.0f) f = 0.0f;
    return f;
}
