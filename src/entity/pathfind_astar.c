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

int astar_find(world *w, vec3 from, vec3 to, astar_path *out) {
    out->length = 0;
    out->current = 0;

    int half = ASTAR_GRID / 2;
    int ox = (int)floorf(from.x) - half;
    int oz = (int)floorf(from.z) - half;
    int oy = (int)floorf(from.y);

    int sx = (int)floorf(from.x) - ox;
    int sz = (int)floorf(from.z) - oz;
    int ex = (int)floorf(to.x)   - ox;
    int ez = (int)floorf(to.z)   - oz;

    // clamp target into grid
    if (ex < 0) ex = 0; if (ex >= ASTAR_GRID) ex = ASTAR_GRID - 1;
    if (ez < 0) ez = 0; if (ez >= ASTAR_GRID) ez = ASTAR_GRID - 1;
    if (sx < 0 || sx >= ASTAR_GRID || sz < 0 || sz >= ASTAR_GRID) return 0;

    // init grid
    memset(grid, 0, sizeof grid);
    for (int z = 0; z < ASTAR_GRID; z++) {
        for (int x = 0; x < ASTAR_GRID; x++) {
            grid[z][x].walkable = walkable_at(w, ox + x, oy, oz + z);
            grid[z][x].parent = -1;
        }
    }

    // open start
    grid[sz][sx].open = 1;
    grid[sz][sx].g = 0;
    grid[sz][sx].h = manh(sx, sz, ex, ez);
    grid[sz][sx].f = grid[sz][sx].h;

    static const int dx[4] = {1, -1, 0, 0};
    static const int dz[4] = {0, 0, 1, -1};

    for (int iter = 0; iter < ASTAR_GRID * ASTAR_GRID; iter++) {
        // find lowest f in open set
        int bx = -1, bz = -1, bf = 999999;
        for (int z = 0; z < ASTAR_GRID; z++) {
            for (int x = 0; x < ASTAR_GRID; x++) {
                if (grid[z][x].open && !grid[z][x].closed && grid[z][x].f < bf) {
                    bf = grid[z][x].f;
                    bx = x; bz = z;
                }
            }
        }
        if (bx < 0) break;

        // reached goal?
        if (bx == ex && bz == ez) {
            // reconstruct path
            int cx = ex, cz = ez;
            int steps[ASTAR_MAX_PATH][2];
            int n = 0;
            while (cx != sx || cz != sz) {
                if (n >= ASTAR_MAX_PATH) break;
                steps[n][0] = cx; steps[n][1] = cz; n++;
                int pi = grid[cz][cx].parent;
                if (pi < 0) break;
                cx = pi % ASTAR_GRID;
                cz = pi / ASTAR_GRID;
            }
            // reverse into output
            for (int i = n - 1; i >= 0; i--) {
                out->nodes[out->length].x = ox + steps[i][0];
                out->nodes[out->length].z = oz + steps[i][1];
                out->nodes[out->length].y = oy;
                out->length++;
            }
            return 1;
        }

        grid[bz][bx].closed = 1;

        for (int d = 0; d < 4; d++) {
            int nx = bx + dx[d];
            int nz = bz + dz[d];
            if (nx < 0 || nx >= ASTAR_GRID || nz < 0 || nz >= ASTAR_GRID) continue;
            if (!grid[nz][nx].walkable || grid[nz][nx].closed) continue;

            int ng = grid[bz][bx].g + 1;
            if (!grid[nz][nx].open || ng < grid[nz][nx].g) {
                grid[nz][nx].g = ng;
                grid[nz][nx].h = manh(nx, nz, ex, ez);
                grid[nz][nx].f = ng + grid[nz][nx].h;
                grid[nz][nx].parent = bz * ASTAR_GRID + bx;
                grid[nz][nx].open = 1;
            }
        }
    }
    return 0;
}

vec3 astar_next_target(const astar_path *path, vec3 entity_pos) {
    if (path->current >= path->length) {
        return entity_pos;
}
    const astar_node *n = &path->nodes[path->current];
;
}
