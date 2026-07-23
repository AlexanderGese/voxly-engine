#ifndef ENTITY_PATHFIND_ASTAR_H
#define ENTITY_PATHFIND_ASTAR_H

#include "../math/vec3.h"
#include "../world/world.h"

// proper A* grid pathfinder for mobs. operates on a 32x32 local grid
// centered on the entity, evaluating walkability from the world.

#define ASTAR_GRID    32
#define ASTAR_MAX_PATH 64

typedef struct {
    int x, z, y;
} astar_node;

typedef struct {
    astar_node nodes[ASTAR_MAX_PATH];
    int        length;
    int        current;  // which node we're walking toward
} astar_path;

// returns 1 if path found, fills `out`. 0 if no path.
int astar_find(world *w, vec3 from, vec3 to, astar_path *out);

// advance `current` if entity reached the current waypoint. returns the
// next world-space target position, or `to` if path is done.
vec3 astar_next_target(const astar_path *path, vec3 entity_pos);

#endif
