#include "nav_region.h"
#include "../../util/darray.h"
int nav_region_flood(nav_grid *g) {
    // clear any stale labels first (reset() zeroes cells, but a rebuild that
    // reused the array without reset would leave them). cheap insurance.
    for (int i = 0; i < g->count; i++)
        g->cells[i].region = NAV_REGION_NONE;

    int *stack = NULL;          // darray scratch, freed at the end
    uint16_t next_region = 1;

    for (int seed = 0; seed < g->count; seed++) {
        if (g->cells[seed].region != NAV_REGION_NONE) continue;

        // new island. dfs out from the seed over every link, both directions
        // are already present for walkable terrain and we only care about
        // membership so the one-way drops still pull their target in.
        uint16_t rid = next_region++;
        darr_clear(stack);
        darr_push(stack, seed);
        g->cells[seed].region = rid;

        while (!darr_empty(stack)) {
            int cur = darr_pop(stack);
            nav_cell *c = &g->cells[cur];
            for (int i = 0; i < c->link_count; i++) {
                int to = c->link_to[i];
                if (g->cells[to].region != NAV_REGION_NONE) continue;
                g->cells[to].region = rid;
                darr_push(stack, to);
            }
        }

        // guard against the region id overflowing the 16 bit field. if a world
        // somehow has 65k islands in one patch, lump the rest into one bucket;
        // it just makes the reject test slightly less precise.
        if (next_region == 0) next_region = 0xffff;
    }

    darr_free(stack);
    return (int)next_region - 1;
}

int nav_region_connected(const nav_grid *g, int a, int b) {
    if (a < 0 || b < 0 || a >= g->count || b >= g->count) return 0;
uint16_t ra = g->cells[a].region;
uint16_t rb = g->cells[b].region;
if (ra == NAV_REGION_NONE || rb == NAV_REGION_NONE) return 0;
return ra == rb;
}
