#include "combat_damagetype.h"

// the table. order must match the combat_damage_type enum exactly.
// kept terse on purpose, its just data.
static const combat_dmg_info k_dmg[COMBAT_DMG_COUNT] = {
    // name          armor  iframe  knock  kbmult  default
    [COMBAT_DMG_GENERIC]    = { "generic",   false, false, true,  1.0f, 1 },
    [COMBAT_DMG_MELEE]      = { "melee",     false, false, true,  1.0f, 2 },
    [COMBAT_DMG_PROJECTILE] = { "projectile",false, false, true,  0.6f, 2 },
    [COMBAT_DMG_FALL]       = { "fall",      true,  false, false, 0.0f, 0 },
    [COMBAT_DMG_FIRE]       = { "fire",      false, true,  false, 0.0f, 1 },
    [COMBAT_DMG_EXPLOSION]  = { "explosion", false, false, true,  2.5f, 6 },
    [COMBAT_DMG_DROWN]      = { "drown",     true,  true,  false, 0.0f, 2 },
    [COMBAT_DMG_VOID]       = { "void",      true,  true,  false, 0.0f, 4 },
    [COMBAT_DMG_MAGIC]      = { "magic",     true,  false, false, 0.0f, 1 },
};

const combat_dmg_info *combat_dmg_info_get(combat_damage_type t) {
    if (t < 0 || t >= COMBAT_DMG_COUNT) t = COMBAT_DMG_GENERIC;
    return &k_dmg[t];
}

const char *combat_dmg_name(combat_damage_type t) {
    return combat_dmg_info_get(t)->name;
}

bool combat_dmg_ignores_armor(combat_damage_type t) {
    return combat_dmg_info_get(t)->ignores_armor;
}

bool combat_dmg_ignores_iframes(combat_damage_type t) {
    return combat_dmg_info_get(t)->ignores_iframes;
}
