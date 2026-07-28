#ifndef ENTITY_EPHYSICS_PUSHOUT_H
#define ENTITY_EPHYSICS_PUSHOUT_H

#include "ephysics_types.h"
#include "ephysics_spatial.h"
#include "../entity.h"

// entity-vs-entity soft separation. mobs shouldnt stack into one square; when
// two bodies overlap we push them apart along the line between their centers,
// weighted by mass so a chicken doesnt shove a player. this is a *soft* push (a
// velocity nudge, not a hard depenetration) so dense crowds relax over a few
// ticks instead of exploding apart in one.
//
// runs after the per-body world step, using the spatial grid to find neighbours.

// horizontal overlap between two entity footprints, meters. <= 0 if clear. we
// treat footprints as circles of radius width/2 for cheap symmetric pushes.
float ephysics_pushout_overlap(const entity *a, const entity *b);

// nudge `a` and `b` apart by adding opposing horizontal velocities. mass-
// weighted: the lighter body moves more. `stiffness` scales the push (0..1,
// ~0.4 feels right). mutates both entities' vel.
void ephysics_pushout_pair(entity *a, entity *b, float stiffness);

// resolve crowding across a whole entity array using the grid. for each live
// entity, query neighbours within a footprint and push apart any overlaps.
// returns the number of pairs it pushed (handy for a "too crowded" stat).
int ephysics_pushout_all(entity *ents, int n, const ephys_grid *g,
                         float stiffness);

#endif
