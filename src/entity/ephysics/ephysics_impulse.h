#ifndef ENTITY_EPHYSICS_IMPULSE_H
#define ENTITY_EPHYSICS_IMPULSE_H

#include "ephysics_types.h"

// impulses: instantaneous velocity changes the physics step has to honour.
// knockback from combat, explosion shoves, jump pads, the recoil when you mine
// a block you were standing in. the combat module computes a knockback *vector*
// already; this is the physics-side that turns force into a velocity delta the
// way the integrator expects, scaled by the body's mass and clamped so a big
// explosion doesnt fling you across the map and tunnel through the world.

// apply an impulse (kg*m/s) to a body: dv = J / mass. mutates b->vel. mass of 0
// is treated as immovable and the impulse is ignored.
void ephysics_impulse_apply(ephys_body *b, vec3 impulse);

// convenience for the common "shove away from a point" case (explosions, melee
// knockback). `strength` is the impulse magnitude at point-blank; it falls off
// linearly to zero at `radius`. adds a little upward kick so bodies pop off the
// ground and the friction pass doesnt immediately eat the shove.
void ephysics_impulse_radial(ephys_body *b, vec3 source, float strength,
                             float radius);

// reflect velocity off a contact normal with the body's restitution. used when
// a bouncy body (dropped item, thrown projectile) hits a surface mid-sweep.
// returns the post-bounce velocity; below `min_speed` it kills the bounce so
// things actually come to rest instead of jittering forever.
vec3 ephysics_impulse_bounce(const ephys_body *b, vec3 vel, vec3 normal,
                             float min_speed);

// hard clamp on total speed after impulses, so a stacked pile of shoves cant
// exceed what the ccd sub-stepper can safely resolve. returns 1 if clamped.
int ephysics_impulse_clamp(ephys_body *b, float max_speed);

#endif
