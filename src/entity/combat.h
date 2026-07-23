#ifndef ENTITY_COMBAT_H
#define ENTITY_COMBAT_H

#include "mob.h"
#include "../player/player.h"
#include "../player/damage.h"

void combat_player_attack(mob_registry *mr, const player *p, int reach_blocks);
void combat_mobs_hit_player(mob_registry *mr, const player *p, damage *d);

#endif
