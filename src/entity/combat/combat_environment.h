#ifndef ENTITY_COMBAT_COMBAT_ENVIRONMENT_H
#define ENTITY_COMBAT_COMBAT_ENVIRONMENT_H

#include "combat_types.h"
#include "combat_hit.h"

// environmental damage: the stuff the world does to you rather than another
// entity. fall, fire-over-time, drowning, the void. these build the right
// kind of combat_hit and route through the normal resolver.

// per-combatant scratch the environment systems need. attach one alongside
// the combatant; the game glue updates the inputs each frame.
typedef struct {
    float fall_start_y;   // y when we left the ground; -inf style sentinel
    int   airborne;       // were we off the ground last frame?

    float burn_timer;     // seconds of fire left. 0 = not burning.
    float burn_tick;      // accumulator toward the next fire tick.

    float air;            // breath, 0..max. drown when it hits 0.
    float air_max;
    float drown_tick;     // accumulator toward the next drown tick.
} combat_env_state;

void combat_env_init(combat_env_state *e, float air_max);

// call when the combatant lands. computes fall distance from the tracked
// start height and applies fall damage if it crossed the threshold.
// returns damage dealt (0 if a soft landing or blocked).
int combat_env_on_land(combat_combatant *c, combat_env_state *e,
                       float land_y, combat_rng *rng);

// call every frame with whether the combatant is currently on the ground,
// so the env state can latch the fall-start height.
void combat_env_track_fall(combat_env_state *e, int on_ground, float y);

// set the combatant on fire for `seconds` (refreshes if already burning).
void combat_env_ignite(combat_env_state *e, float seconds);

// advance burning + drowning. `submerged` says the head is underwater.
// applies ticks of damage through the resolver. call once per frame.
void combat_env_tick(combat_combatant *c, combat_env_state *e,
                     int submerged, float dt, combat_rng *rng);

// instant unblockable void damage for falling out of the world.
void combat_env_void_kill(combat_combatant *c);

#endif
