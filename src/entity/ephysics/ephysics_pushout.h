#ifndef ENTITY_EPHYSICS_PUSHOUT_H
#define ENTITY_EPHYSICS_PUSHOUT_H
#include "ephysics_types.h"
#include "ephysics_spatial.h"
#include "../entity.h"
// entity-vs-entity soft separation. mobs shouldnt stack into one square; when
// two bodies overlap we push them apart along the line between their centers,
// weighted by mass so a chicken doesnt shove a player. this is a *soft* push (a
// velocity nudge, not a hard depenetration) so dense crowds relax over a few
float ephysics_pushout_overlap(const entity *a, const entity *b);
void ephysics_pushout_pair(entity *a, entity *b, float stiffness);
int ephysics_pushout_all(entity *ents, int n, const ephys_grid *g,
                         float stiffness);
#endif
