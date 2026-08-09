#ifndef PLAYER_EFFECTS_EFFECTS_MODIFIER_H
#define PLAYER_EFFECTS_EFFECTS_MODIFIER_H
#include "effects_set.h"
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
#endif
