#ifndef PLAYER_EFFECTS_EFFECTS_POTION_H
#define PLAYER_EFFECTS_EFFECTS_POTION_H
#include "effects_apply.h"
typedef enum {
    POTION_NONE = 0,        // plain water bottle, does nothing
    POTION_AWKWARD,         // brewing base, no effect on its own
    POTION_SWIFTNESS,       // speed
    POTION_SLOWNESS,        // slowness
    POTION_HEALING,         // instant-ish: short strong regen
    POTION_HARMING,         // instant-ish: short strong poison/wither
    POTION_REGEN,           // regeneration
    POTION_POISON,          // poison
    POTION_STRENGTH,        // strength
    POTION_WEAKNESS,        // weakness
    POTION_LEAPING,         // jump boost
    POTION_FIRE_RESIST,     // fire resistance
    POTION_WATER_BREATHING, // water breathing
    POTION_NIGHT_VISION,    // night vision
    POTION_INVISIBILITY,    // invisibility
    POTION_TURTLE_MASTER,   // slowness + resistance combo
    POTION_KIND_COUNT
} effects_potion_kind;
typedef enum {
    POTION_FORM_DRINK = 0,  // full duration, self only
    POTION_FORM_SPLASH,     // ~75% duration, area
    POTION_FORM_LINGERING,  // ~25% per re-application, but reapplies in a cloud
} effects_potion_form;
typedef struct {
    effects_potion_kind kind;
    effects_potion_form form;
    bool extended;          // redstone: longer duration, no level bump
    bool upgraded;          // glowstone: +1 level, shorter duration
} effects_potion;
#endif
