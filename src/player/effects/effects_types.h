#ifndef PLAYER_EFFECTS_EFFECTS_TYPES_H
#define PLAYER_EFFECTS_EFFECTS_TYPES_H

#include <stdint.h>
#include <stdbool.h>

// shared vocabulary for the status-effect subsystem. one header so the rest
// of effects/ doesn't grow a thicket of cross includes (same rationale as
// tools_types.h / combat_types.h).
//
// everything public is prefixed effects_ so it won't trip over the combat_dot
// stuff over in entity/combat — that one chips at health, this one is the
// general buff/debuff bag that sits on the player and warps movement, vision,
// regen, dig speed, etc. they overlap on poison/wither on purpose; the apply
// layer knows to forward damaging kinds to combat where it makes sense.

// the catalogue of effects we know how to run. ordered so the good ones cluster
// at the top — purely cosmetic, the hud doesn't care.
typedef enum {
    EFFECT_NONE = 0,

    EFFECT_SPEED,          // +move speed per level
    EFFECT_SLOWNESS,       // -move speed per level
    EFFECT_HASTE,          // +dig speed (feeds tools_haste)
    EFFECT_FATIGUE,        // -dig speed, mining fatigue
    EFFECT_STRENGTH,       // +melee damage
    EFFECT_WEAKNESS,       // -melee damage
    EFFECT_JUMP_BOOST,     // +jump velocity
    EFFECT_REGENERATION,   // heal a tick every interval
    EFFECT_POISON,         // chip health, floors at 1
    EFFECT_WITHER,         // chip health, DOES kill
    EFFECT_FIRE_RESIST,    // immune to burning
    EFFECT_WATER_BREATHING,// air doesn't drain
    EFFECT_NIGHT_VISION,   // gamma floor
    EFFECT_INVISIBILITY,   // render alpha -> 0
    EFFECT_RESISTANCE,     // flat incoming damage cut
    EFFECT_ABSORPTION,     // bonus soakable health
    EFFECT_SATURATION,     // refill hunger every tick
    EFFECT_HUNGER,         // drain hunger faster
    EFFECT_GLOWING,        // outline, harmless
    EFFECT_LEVITATION,     // gentle upward drift

    EFFECT_KIND_COUNT
} effects_kind;

// broad behaviour class. drives the default particle tint and whether a cure
// item (milk) wipes it. cosmetic effects survive a milk bucket here because
// pulling night-vision off someone for drinking milk felt mean.
typedef enum {
    EFFECT_CAT_BENEFICIAL = 0, // buffs
    EFFECT_CAT_HARMFUL,        // debuffs / dots
    EFFECT_CAT_NEUTRAL,        // glowing, cosmetic-ish
    EFFECT_CAT_COUNT
} effects_category;

// how a fresh application interacts with one already running of the same kind.
typedef enum {
    EFFECT_STACK_REFRESH = 0,  // higher amp or longer dur wins (the common case)
    EFFECT_STACK_ADD_DUR,      // durations sum (saturation-ish)
    EFFECT_STACK_HIGHEST,      // keep strictly the strongest, never extend
} effects_stack_rule;

// duration sentinel. an effect with this many ticks left never expires on its
// own — used for ambient/creative effects. -1 reads cleaner than a huge int.
#define EFFECTS_DURATION_INFINITE  (-1)

// max concurrent effects on one carrier. one slot per kind really, but we keep
// it array-shaped so adding a kind doesn't reshuffle anything.
#define EFFECTS_MAX_ACTIVE         EFFECT_KIND_COUNT

// amplifier ceiling. level 0 == "I", so this is effect level 8. past that the
// numbers stop meaning anything and start overflowing the fun.
#define EFFECTS_MAX_AMPLIFIER       7

// a packed handle to one active effect, handed back by apply for callers that
// want to poke at it later. index into the set + a generation so a stale handle
// can't accidentally address a recycled slot.
typedef struct {
    int16_t  slot;   // -1 == invalid
    uint16_t gen;
} effects_handle;

#define EFFECTS_HANDLE_NONE  ((effects_handle){ -1, 0 })

static inline bool effects_handle_valid(effects_handle h) { return h.slot >= 0; }

#endif
