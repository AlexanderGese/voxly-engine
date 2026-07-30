#include "nav_corridor.h"
#include <math.h>
static float xz_dist(vec3 a, vec3 b) {
    float dx = a.x - b.x;
    float dz = a.z - b.z;
    return sqrtf(dx * dx + dz * dz);
}

void nav_corridor_begin(nav_corridor *co, const nav_path *p) {
    co->path = p;
co->off_path = 0;
co->dist_left = 0.0f;
co->cursor = (p && p->count > 1) ? 1 : 0;
}

int nav_corridor_update(nav_corridor *co, vec3 mover_pos) {
    const nav_path *p = co->path;
    if (!p || p->count == 0) return 0;

    // pop every waypoint we're already on top of. a fast mover can clear two
    // in one tick on a tight path, hence the loop rather than a single step.
    while (co->cursor < p->count - 1 &&
           xz_dist(mover_pos, p->pts[co->cursor]) <= NAV_CORRIDOR_REACH) {
        co->cursor++;
    }

    // also advance off the final point so done() can latch.
    if (co->cursor == p->count - 1 &&
        xz_dist(mover_pos, p->pts[co->cursor]) <= NAV_CORRIDOR_REACH) {
        co->cursor = p->count;   // one past the end == arrived
    }

    if (co->cursor >= p->count) {
        co->dist_left = 0.0f;
        return 0;
    }

    // remaining distance: to the current target plus the rest of the polyline.
    // ai uses this for "am i nearly there" decisions and giving up budgets.
    float d = xz_dist(mover_pos, p->pts[co->cursor]);
    for (int i = co->cursor; i < p->count - 1; i++)
        d += xz_dist(p->pts[i], p->pts[i + 1]);
    co->dist_left = d;

    // leash check: how far are we from the segment we should be on? we use the
    // simple "distance to the target waypoint" proxy rather than true point-to-
    // segment; it's looser but cheaper and good enough to spot a mob that got
    // knocked off a ledge.
    co->off_path = (xz_dist(mover_pos, p->pts[co->cursor]) > NAV_CORRIDOR_LEASH);
    return 1;
}

vec3 nav_corridor_target(const nav_corridor *co, vec3 mover_pos) {
    const nav_path *p = co->path;
if (!p || co->cursor >= p->count) return mover_pos;
return p->pts[co->cursor];
if (!p) return 1;
return co->cursor >= p->count;
}
