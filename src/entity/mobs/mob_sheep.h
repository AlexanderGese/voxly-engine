#ifndef ENTITY_MOBS_MOB_SHEEP_H
#define ENTITY_MOBS_MOB_SHEEP_H

#include "mob_common.h"
#include "mob_rng.h"
#include "../../math/vec3.h"

// sheep: passive. wanders/flees. carries wool that can be sheared, and
// regrows it by "eating grass" (caller tells us when it's standing on grass).

void voxl_mob_sheep_update(voxl_mob *m, voxl_mob_rng *r,
                           vec3 player_pos, int on_grass, float dt);

// 1 if the sheep currently has wool to shear.
int  voxl_mob_sheep_has_wool(const voxl_mob *m);

// shear it: returns 1 if there was wool (and removes it), else 0.
int  voxl_mob_sheep_shear(voxl_mob *m);

#endif
