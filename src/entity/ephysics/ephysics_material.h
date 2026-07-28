#ifndef ENTITY_EPHYSICS_MATERIAL_H
#define ENTITY_EPHYSICS_MATERIAL_H

#include "ephysics_types.h"
#include "../entity.h"

// material presets. these are hand-tuned, dont read too much into the numbers.
// a "material" here is really the whole motion profile of a body, not a surface
// property. naming is a holdover, im not renaming it now.

ephys_material ephysics_material_default(void);
ephys_material ephysics_material_player(void);
ephys_material ephysics_material_item(void);     // dropped items, bouncy-ish
ephys_material ephysics_material_for(entity_type t);

// build a body from an entity. pulls width/height off the entity helpers and
// seeds a sensible material. caller still owns the entity; we just snapshot it.
ephys_body ephysics_body_from_entity(const entity *e);

// write the integrated body state back onto the entity (pos, vel, on_ground).
void ephysics_body_to_entity(const ephys_body *b, entity *e);

#endif
