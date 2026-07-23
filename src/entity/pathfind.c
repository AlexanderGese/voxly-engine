#include "pathfind.h"
#include "../world/block.h"
#include "../config.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
// 16x16 local grid, center = entity position. cheap heuristic A*.
// this is the "started but never finished" module. mobs dont use it yet.
#define GRID 16
typedef struct {
    int  parent;
    int  g;
    int  f;
    int  closed;
    int  open;
} cell;
static int manh(int x1, int z1, int x2, int z2) {
    return abs(x1 - x2) + abs(z1 - z2);
}

int pathfind(world *w, vec3 from, vec3 to, path *out) {
    (void)w;
(void)from;
(void)to;
(void)out;
// TODO: actually implement this. stub for now, returns 0.
memset(out, 0, sizeof *out);
return 0;
}
