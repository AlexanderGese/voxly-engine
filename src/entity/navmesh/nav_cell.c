#include "nav_cell.h"
#include "../../world/block.h"

// water/lava are the only fluids the engine has; both are BLOCK_WATER-ish in
// terms of "agent won't stand on it". keep it simple: solid = floor, air =
// clearance, everything transparent-but-not-solid (glass aside) we treat as
// not-floor. glass is solid so it's a valid floor, which is fine.

static int is_fluid(block_id id) {
    // only water for now. if lava ever lands it goes here too.
    return id == BLOCK_WATER;
}

int nav_cell_passable(world *w, int wx, int wy, int wz) {
    block_id id = world_get_block(w, wx, wy, wz);
    if (block_is_solid(id)) return 0;
    if (is_fluid(id)) return 0;   // don't route mobs into deep water
    return 1;
}

int nav_cell_standable(world *w, int wx, int wy, int wz) {
    block_id floor = world_get_block(w, wx, wy, wz);
    if (!block_is_solid(floor)) return 0;

    // need AGENT_HEIGHT clear blocks above the floor for the body to fit.
    for (int h = 1; h <= NAV_AGENT_HEIGHT; h++) {
        if (!nav_cell_passable(w, wx, wy + h, wz)) return 0;
    }
    return 1;
}

int nav_cell_spans(world *w, int wx, int wz, int y_lo, int y_hi,
                   int16_t *out, int max) {
    if (y_lo < 0) y_lo = 0;
    int n = 0;

    int y = y_lo;
    while (y <= y_hi && n < max) {
        if (nav_cell_standable(w, wx, y, wz)) {
            out[n++] = (int16_t)y;
            // we found a floor at y; the body occupies y+1..y+H. there can't
            // be another floor inside the body, so skip past it. the next
            // possible floor is the first solid block above the head.
            y += NAV_AGENT_HEIGHT + 1;
        } else {
            y++;
        }
    }
    return n;
}
