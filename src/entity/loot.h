#ifndef ENTITY_LOOT_H
#define ENTITY_LOOT_H

#include "../world/item.h"
#include "entity.h"

// when a mob dies, drop loot items at its position.

void loot_drop(item_world *iw, const entity *e);

#endif
