#ifndef ENTITY_PATHFIND_H
#define ENTITY_PATHFIND_H

#include "../math/vec3.h"
#include "../world/world.h"

// 2d grid A* in a small window centered on the entity. produces a list
// of waypoints the AI can walk along. not used by the basic mobs yet
// but reserved for smarter behavior.

#define PATH_MAX 64

typedef struct {
    int x, z, y;
} path_node;

typedef struct {
    path_node nodes[PATH_MAX];
    int       count;
} path;

int pathfind(world *w, vec3 from, vec3 to, path *out);

#endif
