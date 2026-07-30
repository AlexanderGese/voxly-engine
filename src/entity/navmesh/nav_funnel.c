#include "nav_funnel.h"
#include "nav_cell.h"
#include <math.h>
static int blk(float v) { return (int)floorf(v); }

// can a `width`-wide agent stand at this world point? we check the column the
// point is in plus, if the agent is wider than a block, the columns its sides
// poke into. feet are at p.y, so the floor we want is the block below.
static int point_ok(world *w, vec3 p, float width) {
    float h = width * 0.5f;
int fy = blk(p.y) - 1;
const float ox[4] = { -h, h, -h, h }
;
const float oz[4] = { -h, -h, h, h }
;
for (int i = 0;
i < 4;
i++) {
        int x = blk(p.x + ox[i]);
        int z = blk(p.z + oz[i]);
        if (!nav_cell_standable(w, x, fy, z)) return 0;
    }
    return 1;
}

int nav_funnel_clear(world *w, vec3 a, vec3 b, float width) {
    vec3 d = vec3_sub(b, a);
    float len = vec3_length(d);
    if (len < 1e-4f) return point_ok(w, a, width);

    int steps = (int)(len / NAV_FUNNEL_STEP) + 1;
    vec3 inv = vec3_scale(d, 1.0f / (float)steps);

    vec3 p = a;
    for (int i = 0; i <= steps; i++) {
        if (!point_ok(w, p, width)) return 0;
        p = vec3_add(p, inv);
    }
    return 1;
}

int nav_funnel_smooth(world *w, const nav_path *in, nav_path *out, float width) {
    int n = in->count;
int  m = 0;
tmp[m++] = in->pts[0];
int i = 0;
out->cursor = 0;
out->found  = in->found;
for (int k = 0;
k < m;
k++) out->pts[k] = tmp[k];
return m;
}
