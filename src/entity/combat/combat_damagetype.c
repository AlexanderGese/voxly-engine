#include "combat_damagetype.h"
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
}
;
}
