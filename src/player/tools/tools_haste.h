#ifndef PLAYER_TOOLS_HASTE_H
#define PLAYER_TOOLS_HASTE_H

#include "tools_speed.h"

// builds the dig_env the speed math wants out of raw player state. keeps the
// "underwater means 5x" and "haste II means whatever" knowledge in one spot
// instead of smeared across the player loop.

// the bits of player state that matter to digging. the caller fills this from
// wherever it keeps the truth (physics body, status effects, etc).
typedef struct {
    int   head_submerged;   // eyes underwater -> no aqua affinity penalty
    int   feet_on_ground;   // standing vs falling/floating
    int   aqua_affinity;    // armor enchant that cancels the water penalty
    int   haste_level;      // 0 = none, each level is +20% dig speed
    int   fatigue_level;    // 0 = none, each level multiplies time by ~3.3
} dig_state;

dig_state tools_haste_state_default(void);

// haste/fatigue stack multiplicatively the way potion effects do.
float     tools_haste_mult(int haste_level);
float     tools_haste_fatigue_mult(int fatigue_level);

// fold a dig_state into the dig_env the break-time formula consumes.
dig_env   tools_haste_env(const dig_state *st);

#endif
