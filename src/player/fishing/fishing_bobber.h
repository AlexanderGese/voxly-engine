#ifndef PLAYER_FISHING_BOBBER_H
#define PLAYER_FISHING_BOBBER_H

#include "fishing_types.h"
#include "../../world/world.h"

// the bobber sim. an arc out of the rod, a settle onto the water, an idle bob,
// and a yank toward the rod on reel-in. no rendering here, just positions.

// launch a bobber from `origin` along `dir` (normalised look vector). power is
// the rod's cast_power; the arc reads it as an initial speed.
void fishing_bobber_cast(fishing_bobber *b, vec3 origin, vec3 dir, float power);

// integrate one step. while airborne it falls under gravity until it hits
// water (lands) or a solid block (also lands, but water stays WATER_NONE).
// once landed it floats and bobs. dt in seconds.
void fishing_bobber_update(fishing_bobber *b, world *w, float dt);

// pull the bobber toward `target` (the rod tip) at `speed`. used during reel.
// returns 1 once it's within grab range, meaning the catch arrives.
int  fishing_bobber_reel_toward(fishing_bobber *b, vec3 target, float speed, float dt);

// nudge the submerge value toward `target` so a bite can visibly dunk it.
void fishing_bobber_set_dunk(fishing_bobber *b, float target, float dt);

#endif
