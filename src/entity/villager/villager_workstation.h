#ifndef ENTITY_VILLAGER_WORKSTATION_H
#define ENTITY_VILLAGER_WORKSTATION_H

#include "villager.h"
#include "villager_poi.h"
#include "../../world/world.h"

// jobs layer: matches villagers to workstation POIs and keeps a villager's
// claimed station honest (still there, still the right block). this is the
// bridge between the abstract poi registry and a villager actually having a
// profession.

// scan the world around `origin` and register any workstation/bed/bell blocks
// as POIs. cheap box scan; call occasionally, not every tick. returns number
// of new POIs registered.
int villager_workstation_scan(villager_poi_set *pois, world *w,
                              vec3 origin, int radius);

// try to give an unemployed (or baby-grown) villager a job: find a free
// workstation in range, claim it, and switch the villager's profession to
// match the block. returns 1 if employed, 0 if nothing available.
int villager_workstation_seek_job(villager *v, villager_poi_set *pois, vec3 from);

// verify the villager's claimed station still exists and is the right block.
// if it vanished, release it and demote to unemployed. returns 1 if still ok.
int villager_workstation_validate(villager *v, villager_poi_set *pois, world *w);

// claim the nearest free bed for sleeping. returns 1 on success.
int villager_workstation_claim_bed(villager *v, villager_poi_set *pois, vec3 from);

#endif
