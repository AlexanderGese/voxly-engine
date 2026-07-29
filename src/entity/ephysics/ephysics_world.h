#ifndef ENTITY_EPHYSICS_WORLD_H
#define ENTITY_EPHYSICS_WORLD_H
#include "ephysics_types.h"
#include "ephysics_integrate.h"
#include "ephysics_contact.h"
#include "../entity.h"
#include "../../world/world.h"
// batch tick driver. ephysics_step_body handles one body; this walks a whole
// entity array once per fixed step, runs each through the integrator, and
// surfaces the per-body landing/impact events so the game glue can react
// (footstep sfx, fall damage, splash particles) without re-deriving them.
//
// it also keeps a tiny per-frame stat block which the f3 overlay reads. nothing
// here owns the entities; the caller passes its own array and we mutate in place.
typedef struct {
    uint32_t entity_id;
    float    land_speed;    // downward closing speed on touchdown, 0 if airborne
    float    impact_speed;  // hardest contact this tick, any axis
    uint32_t flags;         // the body's EPHYS_F_* after the step
    int      entered_water; // crossed a fluid surface this tick (splash trigger)
} ephys_event;
typedef struct {
    int   bodies;          // how many we ticked
    int   grounded;        // how many ended grounded
    int   in_fluid;        // how many overlapped fluid
    int   events;          // events emitted
    float max_speed_seen;  // fastest body this tick, m/s
} ephys_world_stats;
int ephysics_world_tick(world *w, entity *ents, int n,
                        const ephys_input *inputs, float dt,
                        ephys_event *out_events, int out_cap,
                        ephys_world_stats *stats);
int ephysics_world_tick_one(world *w, entity *e, const ephys_input *in, float dt,
                            ephys_event *out);
#endif
