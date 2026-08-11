#ifndef PLAYER_FISHING_FX_H
#define PLAYER_FISHING_FX_H

#include "fishing_types.h"
#include "fishing_rng.h"
#include "../../fx/particles.h"

// cosmetic splashes. piggybacks on the engine's existing cpu particle system —
// we just push blue droplets into its pool the same way the block-break dust
// does. all event-driven; the session calls these at the right moments. if no
// particle_system is wired in, every call is a harmless no-op.

// the splash when the bobber first hits the water on a cast.
void fishing_fx_splash(particle_system *ps, fishing_rng *r, vec3 at);

// the gentle ring of droplets while a fish is approaching (LURING phase). call
// it sparingly — it spawns only a couple particles per call by design.
void fishing_fx_lure_ripple(particle_system *ps, fishing_rng *r, vec3 at, float intensity);

// the sharp downward plunk the instant a fish bites (BITING phase).
void fishing_fx_bite(particle_system *ps, fishing_rng *r, vec3 at);

// a triumphant little burst when a catch is landed.
void fishing_fx_land(particle_system *ps, fishing_rng *r, vec3 at, fishing_catch_category cat);

#endif
