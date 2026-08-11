#ifndef PLAYER_FISHING_SESSION_H
#define PLAYER_FISHING_SESSION_H

#include "fishing_types.h"
#include "fishing_rng.h"
#include "fishing_rod.h"
#include "fishing_bobber.h"
#include "fishing_bite.h"
#include "fishing_reel.h"
#include "fishing_loot.h"
#include "fishing_catch.h"
#include "fishing_stats.h"
#include "fishing_fx.h"
#include "../../world/world.h"
#include "../../world/item.h"

// the whole thing wired together. one session per rod-in-hand. the player input
// is two edges: a cast/strike press (toggles the line) and a held reel button.
// the machine walks IDLE -> FLYING -> WAITING -> LURING -> BITING -> HOOKED ->
// REELING -> back to IDLE, with SNAPPED as the failure exit.

typedef struct {
    fishing_cast_state state;
    fishing_rod        rod;
    fishing_bobber     bobber;
    fishing_bite       bite;
    fishing_reel       reel;
    fishing_rng        rng;

    fishing_catch      pending;     // the roll waiting to be reeled in
    vec3               rod_tip;     // updated each frame from the player
    float              quality;     // cached water quality at the cast site
    int                active;      // is a line in the water at all
    fishing_stats     *stats;       // optional career tracker; may be NULL
    particle_system   *fx;          // optional splash effects; may be NULL
} fishing_session;

// set up a session with a rod and a seed. starts IDLE.
void fishing_session_init(fishing_session *s, fishing_rod rod, uint64_t seed);

// attach a career stats tracker. pass NULL to detach. purely for the readout;
// nothing in the core loop depends on it.
void fishing_session_set_stats(fishing_session *s, fishing_stats *stats);

// attach a particle system for splash effects. pass NULL to disable. cosmetic
// only; the simulation is identical with or without it.
void fishing_session_set_fx(fishing_session *s, particle_system *fx);

// the player pressed the action button. casts if idle, or strikes/reels if a
// line is out. returns the new state for convenience.
fishing_cast_state fishing_session_action(fishing_session *s, vec3 origin, vec3 dir);

// per-frame tick. `reeling` is 1 while the reel button is held (only matters in
// HOOKED). `iw` may be NULL if you don't want loot spawned into the world yet.
void fishing_session_update(fishing_session *s, world *w, item_world *iw,
                            vec3 rod_tip, int reeling, float dt);

// reel everything in and go idle without awarding loot. for swapping items etc.
void fishing_session_cancel(fishing_session *s);

#endif
