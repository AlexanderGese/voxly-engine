#ifndef ENTITY_MOBS_MOB_SPIDER_H
#define ENTITY_MOBS_MOB_SPIDER_H

#include "mob_common.h"
#include "mob_rng.h"
#include "../../math/vec3.h"

// spider: fast erratic melee hostile. lunges at the player in bursts and
// pauses between pounces. only aggressive in the "dark" (caller decides).

// `is_dark` gates hostility (spiders are passive in daylight). returns the
// melee damage dealt this frame, 0 otherwise.
int voxl_mob_spider_update(voxl_mob *m, voxl_mob_rng *r,
                           vec3 player_pos, int is_dark, float dt);

#endif
