#ifndef PLAYER_SWIM_H
#define PLAYER_SWIM_H

#include "player.h"

// returns 1 if the player's head is in water.
int  swim_is_in_water(const player *p, world *w);

// dampens velocity and allows jump-to-surface while in water.
void swim_apply(player *p, world *w, float dt);

#endif
