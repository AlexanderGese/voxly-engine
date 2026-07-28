#ifndef ENTITY_EPHYSICS_INTEGRATE_H
#define ENTITY_EPHYSICS_INTEGRATE_H

#include "ephysics_types.h"
#include "../entity.h"
#include "../../world/world.h"

// the per-tick integrator. wires the whole pipeline together:
// apply gravity -> fluid buoyancy/drag -> clamp -> sweep+resolve -> step-up
// -> friction -> grounding recheck.
// callers feed a body, the world, and dt. everything mutates `b` in place.

// optional wish-velocity input from AI / player controller, in m/s, xz only.
// this is *desired* horizontal velocity; the integrator blends toward it with
// an acceleration so movement isnt instant-on. pass VEC3_ZERO for "no input".
typedef struct {
    vec3  wish_vel;     // desired horizontal velocity (xz)
    float accel;        // how fast we approach wish_vel, m/s^2
    int   jump;         // 1 to request a jump this tick
    int   swim_up;      // 1 to paddle upward while in water
} ephys_input;

ephys_input ephysics_input_none(void);

// one fixed step. dt should be a fixed timestep (the engine ticks at a fixed
// rate); feeding variable dt works but step-up/grounding get jittery.
void ephysics_step_body(world *w, ephys_body *b, const ephys_input *in, float dt);

// drive an entity directly: snapshot -> integrate -> write back. convenience
// wrapper most callers will use instead of touching ephys_body. `in` may be
// NULL for passive bodies (falling items, ragdolls) that just want gravity.
void ephysics_tick_entity(world *w, entity *e, const ephys_input *in, float dt);

#endif
