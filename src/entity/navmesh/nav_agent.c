#include "nav_agent.h"

nav_agent nav_agent_default(void) {
    nav_agent a;
    a.height    = NAV_AGENT_HEIGHT;
    a.step_up   = NAV_STEP_UP;
    a.step_down = NAV_STEP_DOWN;
    a.jump_down = NAV_JUMP_DOWN;
    return a;
}

nav_agent nav_agent_short(void) {
    nav_agent a = nav_agent_default();
    a.height = 1;          // fits under a one-block ceiling
    return a;
}

nav_agent nav_agent_tall(void) {
    nav_agent a = nav_agent_default();
    a.height    = 3;       // needs a taller hole to pass
    a.step_up   = 1;       // big things don't hop fences
    a.jump_down = 3;       // and won't fling themselves off cliffs
    return a;
}

nav_agent nav_agent_sane(nav_agent a) {
    // clamp to ranges the builder + the 16-bit cell coords can actually
    // represent. a zero height would mark solid blocks walkable, which is the
    // kind of bug you chase for an hour.
    if (a.height < 1)    a.height = 1;
    if (a.height > 8)    a.height = 8;
    if (a.step_up < 0)   a.step_up = 0;
    if (a.step_up > 4)   a.step_up = 4;
    if (a.step_down < 0) a.step_down = 0;
    if (a.step_down > 8) a.step_down = 8;
    // a jump-down shorter than the walk-down rule is nonsense; floor it there.
    if (a.jump_down < a.step_down) a.jump_down = a.step_down;
    if (a.jump_down > 16) a.jump_down = 16;
    return a;
}

int nav_agent_eq(nav_agent a, nav_agent b) {
    return a.height    == b.height
        && a.step_up   == b.step_up
        && a.step_down == b.step_down
        && a.jump_down == b.jump_down;
}
