#ifndef ENTITY_EPHYSICS_DEPEN_H
#define ENTITY_EPHYSICS_DEPEN_H

#include "ephysics_types.h"
#include "ephysics_broadphase.h"

// depenetration. the sweep deliberately refuses to report a hit when a body
// *starts* a tick already overlapping a block (it'd wedge at t=0), and trusts
// "the depenetration pass" to fix it. this is that pass. it happens when the
// world changes under a body: a block placed inside you, a chunk popping in, a
// piston shoving you. we find the cheapest axis to escape on and push out along
// it, one block at a time, until clear.
//
// it runs before the sweep each step. on a quiet tick (no overlap) it's a couple
// of cheap aabb tests and returns immediately.

// is the body currently overlapping any solid candidate box. fast reject so the
// integrator can skip the resolve when we're clean.
int ephysics_depen_overlapping(const ephys_candidates *c, const ephys_body *b);

// push `b` out of every overlapping box along the minimum-translation axis.
// iterates up to `max_iters` because escaping one box can still leave you inside
// a neighbour. mutates b->pos. returns the total distance pushed (0 if clean).
float ephysics_depen_resolve(const ephys_candidates *c, ephys_body *b,
                             int max_iters);

// emergency "i'm buried" escape: when a body is overlapping on every axis and
// the mtv is ambiguous (e.g. spawned inside a wall), shove it straight up until
// it clears or we give up after `max_up` meters. returns distance moved.
float ephysics_depen_eject_up(world *w, ephys_body *b, float max_up);

#endif
