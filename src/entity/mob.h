#ifndef ENTITY_MOB_H
#define ENTITY_MOB_H

#include "entity.h"
#include "../world/world.h"
#include "../player/player.h"

// mob registry: holds a flat array of alive entities. cheap.

#define MAX_MOBS 128

typedef struct {
    entity list[MAX_MOBS];
    int    count;
} mob_registry;

void mob_registry_init(mob_registry *mr);
int  mob_spawn(mob_registry *mr, entity_type t, vec3 pos);
void mob_remove(mob_registry *mr, uint32_t id);
void mob_update_all(mob_registry *mr, world *w, const player *p, float dt);

entity *mob_by_id(mob_registry *mr, uint32_t id);

#endif
