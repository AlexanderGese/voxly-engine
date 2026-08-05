#ifndef ENTITY_VILLAGER_PATHING_H
#define ENTITY_VILLAGER_PATHING_H

#include "villager.h"
#include "../pathfind_astar.h"
#include "../../world/world.h"
#include "../../math/vec3.h"

// movement layer for villagers. wraps the engine's grid A* so the brain can
// say "go stand on that block" and not think about waypoints. holds the
// current path and steers the villager along it with simple seek + gravity.

typedef struct {
    astar_path path;
    vec3       goal;        // world-space block we're heading to
    int        has_goal;
    int        stuck_count; // bumps when we make no progress; triggers repath
    float      last_dist;   // distance to current waypoint last tick
} villager_nav;

void villager_nav_reset(villager_nav *nav);

// request a path to `goal`. recomputes only if goal moved or we have none.
// returns 1 if a usable path exists afterward.
int  villager_nav_set_goal(villager_nav *nav, world *w, vec3 from, vec3 goal);

// step the villager one tick toward its goal. applies velocity + a cheap
// gravity/ground snap. returns 1 once the villager has effectively arrived.
int  villager_nav_advance(villager_nav *nav, villager *v, world *w, float dt);

// have we got somewhere to go right now?
int  villager_nav_has_goal(const villager_nav *nav);

#endif
