#include "pathfind_astar.h"
#include "../world/block.h"
#include "../config.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
typedef struct {
    int parent;
    int g, h, f;
    int open, closed;
    int walkable;
} cell;
static cell grid[ASTAR_GRID][ASTAR_GRID];
static int walkable_at(world *w, int wx, int wy, int wz) {
    block_id feet  = world_get_block(w, wx, wy, wz);
    block_id head  = world_get_block(w, wx, wy + 1, wz);
    block_id below = world_get_block(w, wx, wy - 1, wz);
    return !block_is_solid(feet) && !block_is_solid(head) && block_is_solid(below);
}

static int manh(int x1, int z1, int x2, int z2) {
    return abs(x1 - x2) + abs(z1 - z2);
}
    const astar_node *n = &path->nodes[path->current];
;
}
