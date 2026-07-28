#ifndef ENTITY_EPHYSICS_FALLDMG_H
#define ENTITY_EPHYSICS_FALLDMG_H

#include "ephysics_types.h"
#include "ephysics_contact.h"

// fall damage. physics doesnt deal damage itself (that's combat's job) but it
// owns the *kinematics* of it: tracking how far a body has fallen and turning a
// landing impact into a damage amount the combat layer applies. keeping the
// curve here means the threshold stays in sync with GRAVITY/jump tuning.
//
// the integrator can run sub-steps, so a single tick's land_speed isnt always
// the whole story; we keep a tiny accumulator that follows a body across ticks
// and resets on touchdown.

// per-body fall tracker. one of these lives alongside each entity that can take
// fall damage. cheap, two floats.
typedef struct {
    float fall_start_y;   // y where the current fall began
    float peak_speed;     // fastest downward speed seen this fall, m/s
    int   falling;        // 1 while airborne and descending
} ephys_fall;

void ephysics_fall_reset(ephys_fall *f);

// feed one post-step body state. updates the tracker: starts a fall when the
// body leaves the ground descending, records peak speed, and arms a landing.
void ephysics_fall_update(ephys_fall *f, const ephys_body *b);

// distance fallen so far in the current fall, meters. 0 if not falling.
float ephysics_fall_distance(const ephys_fall *f, const ephys_body *b);

// on a touchdown, convert the accumulated fall into damage points. returns 0
// for a safe landing (under the safe-fall threshold) or when not landing this
// tick. water/soft landings should be vetoed by the caller via `cushioned`.
// also clears the tracker so the next fall starts fresh.
int ephysics_fall_damage(ephys_fall *f, const ephys_body *b, int cushioned);

#endif
