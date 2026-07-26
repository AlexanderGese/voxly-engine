#ifndef ENTITY_ECS_PREFAB_H
#define ENTITY_ECS_PREFAB_H

#include "ecs_world.h"
#include "../../math/vec3.h"

// prefabs: canned component bundles so callers dont hand-assemble eight
// ecs_add() calls every spawn. these mirror the entity_type kinds from the old
// entity.h but build them out of components instead of one fat struct. as the
// game migrates, spawner.c will call these instead of entity_new().

// a generic mob: transform + velocity + collider + health + ai + tag.
// `kind` is an entity_type value. width/height/hp come from the caller because
// the per-type table still lives over in entity.c and i'm not duplicating it.
ecs_entity ecs_prefab_mob(ecs_world *w, uint16_t kind, vec3 pos,
                          float width, float height, int max_hp);

// a dropped item: small collider, gravity, a render model, and a lifetime so it
// despawns after a while like every voxel game's ground loot does.
ecs_entity ecs_prefab_item_drop(ecs_world *w, uint16_t item_id, vec3 pos,
                                float lifetime_s);

// a static decoration / marker: just a transform + renderable, no physics. used
// for spawn points, particle anchors, that kind of thing.
ecs_entity ecs_prefab_marker(ecs_world *w, vec3 pos, uint16_t model_id);

#endif
