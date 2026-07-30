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
if (!p || co->cursor >= p->count) return mover_pos;
return p->pts[co->cursor];
if (!p) return 1;
return co->cursor >= p->count;
}
