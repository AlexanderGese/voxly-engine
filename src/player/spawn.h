#ifndef PLAYER_SPAWN_H
#define PLAYER_SPAWN_H

#include "../math/vec3.h"
#include "../world/world.h"

// finds a safe spawn position (top of terrain + eye room) near (x, z).
vec3 spawn_find(world *w, int wx, int wz);

#endif
