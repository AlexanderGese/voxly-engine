#ifndef UI_DEBUG_H
#define UI_DEBUG_H

#include "../player/player.h"
#include "../world/world.h"

void debug_toggle(void);
int  debug_visible(void);

// called every frame. prints stats to stderr since we have no text render
void debug_frame(const player *p, const world *w, float dt);

#endif
