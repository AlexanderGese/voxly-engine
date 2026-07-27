#ifndef ENTITY_EPHYSICS_CONTACT_H
#define ENTITY_EPHYSICS_CONTACT_H
#include "ephysics_types.h"
// contact manifold for a single tick. the resolver in ephysics_sweep produces
// one ephys_hit per slide iteration and then throws them away. plenty of game
// code wants to know *what* we touched and *how hard*: landing sounds, fall
// damage, footstep particles tied to the surface, "did i just hit a wall hard
// enough to take impact damage". so we collect the contacts as they happen.
//
// this is a small fixed-size accumulator; you reset it at the top of a step,
// the resolver/step code adds to it, then the glue reads it back out.
#define EPHYS_MAX_CONTACTS 8
typedef struct {
    ephys_axis axis;       // which axis the contact was on
    vec3       normal;     // contact normal, points back at the body
    float      impact;     // closing speed along the normal at contact, m/s
    float      t;          // fraction of the move when it happened (0..1)
} ephys_contact;
typedef struct {
    ephys_contact list[EPHYS_MAX_CONTACTS];
    int   count;
    float max_impact;      // largest impact magnitude this tick, convenience
    int   dropped;         // contacts past the cap. informational only
} ephys_manifold;
// clear for a fresh tick.
void ephysics_manifold_reset(ephys_manifold *m);
// record one contact. `pre_vel` is the body velocity *before* the resolver
// zeroed the blocked component, so we can recover the impact speed. silently
// drops past the cap (and bumps `dropped`).
void ephysics_manifold_add(ephys_manifold *m, const ephys_hit *h, vec3 pre_vel);
// did we land on a floor this tick (any contact with normal.y > 0).
int ephysics_manifold_grounded(const ephys_manifold *m);
// vertical closing speed of the hardest floor contact, 0 if we didnt land.
// this is the number fall-damage wants.
float ephysics_manifold_land_speed(const ephys_manifold *m);
#endif
