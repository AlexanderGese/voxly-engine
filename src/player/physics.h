#ifndef PLAYER_PHYSICS_H
#define PLAYER_PHYSICS_H

#include "player.h"

// axis-separated movement with block aabb collision
void physics_move_player(player *p, world *w, float dt);

#endif
