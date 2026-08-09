#ifndef PLAYER_EFFECTS_EFFECTS_POTION_H
#define PLAYER_EFFECTS_EFFECTS_POTION_H

#include "effects_apply.h"

// potions: the bridge between an item the player drinks/throws and the effects
// it grants. a potion is a recipe id plus two modifier bits (extended duration,
// boosted level) — the same shape a brewing stand twiddles. drinking one walks
// its effect list through peffects_apply.

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

// how a potion is delivered. affects duration (splash potions land shorter).
typedef enum {
    POTION_FORM_DRINK = 0,  // full duration, self only
    POTION_FORM_SPLASH,     // ~75% duration, area
    POTION_FORM_LINGERING,  // ~25% per re-application, but reapplies in a cloud
} effects_potion_form;

// a concrete potion instance: recipe + the brewing-stand toggles.
typedef struct {
    effects_potion_kind kind;
    effects_potion_form form;
    bool extended;          // redstone: longer duration, no level bump
    bool upgraded;          // glowstone: +1 level, shorter duration
} effects_potion;

// build a plain drinkable potion of a kind.
effects_potion effects_potion_make(effects_potion_kind kind);

// human name for hud / tooltip.
const char *effects_potion_name(effects_potion_kind k);

// apply every effect a potion carries to a target set. `im` may be NULL.
// returns how many of its effects actually took. `splash_strength` 0..1 scales
// a splash potion by distance from the impact (1 == direct hit); ignored for
// drink form.
int effects_potion_drink(effects_set *s, const effects_immunity *im,
                         const effects_potion *p, uint32_t source_id);
int effects_potion_splash(effects_set *s, const effects_immunity *im,
                          const effects_potion *p, float splash_strength,
                          uint32_t source_id);

#endif
