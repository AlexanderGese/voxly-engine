#ifndef PLAYER_EFFECTS_EFFECTS_MODIFIER_H
#define PLAYER_EFFECTS_EFFECTS_MODIFIER_H

#include "effects_set.h"

// turns the active-effect bag into a flat block of derived numbers the rest of
// the game multiplies into its own state. the player loop reads this once a
// frame and feeds the pieces to physics (speed/jump), tools (dig), combat
// (damage/resist) and render (gamma/alpha). no system pokes the effect set
// directly — they all go through this snapshot.

typedef struct {
    // multipliers, 1.0 == unaffected. fold these straight onto the base values
    // from config.h (PLAYER_MOVE_SPEED, PLAYER_JUMP_VEL, ...).
    float move_speed_mult;
    float jump_vel_mult;
    float dig_speed_mult;     // for tools_haste's haste/fatigue lanes
    float melee_damage_mult;
    float incoming_damage_mult; // resistance: < 1 means you take less

    // additive bonuses.
    int   absorption_hp;      // extra soakable health on top of max

    // render hints, 0..1.
    float gamma_floor;        // night vision raises this
    float render_alpha;       // invisibility drops this toward 0

    // booleans the host treats as gates.
    bool  fire_immune;        // skip fire/lava ticks
    bool  water_breathing;    // don't drain air
    bool  glowing;            // outline pass
    bool  levitating;         // physics applies gentle lift instead of gravity
    float levitation_accel;   // upward accel (m/s^2) when levitating
} effects_modifiers;

// neutral state — what you get with no effects at all.
effects_modifiers effects_modifier_identity(void);

// fold the whole set down into one modifier block.
effects_modifiers effects_modifier_compute(const effects_set *s);

// the two lanes tools_haste cares about, pulled out so the dig math doesn't
// need the whole struct. levels, not multipliers (0 == none).
int effects_modifier_haste_level(const effects_set *s);
int effects_modifier_fatigue_level(const effects_set *s);

#endif
