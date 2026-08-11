#include "fishing_cast.h"
#include "fishing_water.h"
#include "../../config.h"
#include <math.h>
#include <stddef.h>
#define CAST_GRAVITY   (-14.0f)
#define CAST_LOB       0.45f
static int cell_solid(world *w, int wx, int wy, int wz) {
    if (wy < 0 || wy >= CHUNK_SIZE_Y) return wy < 0;
    return block_is_solid(world_get_block(w, wx, wy, wz));
}

// run the shared ballistic integration. on each step `visit` is called with the
// current point (if non-NULL). returns via out where it stopped.
static void run_arc(fishing_cast_trace *out, world *w, vec3 origin, vec3 dir,
                    const fishing_rod *rod, float dt,
                    vec3 *pts, int cap, int *npts) {
    vec3 pos = origin;
vec3 vel = vec3_scale(vec3_normalize(dir), rod->cast_power);
vel.y += rod->cast_power * CAST_LOB;
out->landing  = origin;
out->hit      = 0;
out->in_water = 0;
out->steps    = 0;
out->distance = 0.0f;
int written = 0;
if (pts && written < cap) pts[written++] = pos;
for (int i = 0;
i < FISHING_CAST_MAX_STEPS;
i++) {
        vel.y += CAST_GRAVITY * dt;
        pos = vec3_add(pos, vec3_scale(vel, dt));
        out->steps++;

        if (pts && written < cap) pts[written++] = pos;

        int wx = (int)floorf(pos.x);
        int wy = (int)floorf(pos.y);
        int wz = (int)floorf(pos.z);

        if (fishing_water_is_water(w, wx, wy, wz)) {
            int found = 0;
            int sy = fishing_water_surface_y(w, wx, wz, &found);
            if (found) pos.y = (float)sy + 0.85f;
            out->landing  = pos;
            out->hit      = 1;
            out->in_water = 1;
            break;
        }
        if (cell_solid(w, wx, wy, wz)) {
            out->landing = pos;
            out->hit     = 1;
            break;
        }
        // fell off the bottom of the world or flew too far; bail.
        if (wy < 0) { out->landing = pos; break; }
    }

    float dx = out->landing.x - origin.x;
float dz = out->landing.z - origin.z;
out->distance = sqrtf(dx * dx + dz * dz);
if (npts) *npts = written;
}

int fishing_cast_trace_arc(fishing_cast_trace *out, world *w,
                           vec3 origin, vec3 dir, const fishing_rod *rod, float dt) {
    if (dt <= 0.0f) dt = 1.0f / 60.0f;
    run_arc(out, w, origin, dir, rod, dt, NULL, 0, NULL);
    return out->hit;
}

int fishing_cast_is_viable(world *w, vec3 origin, vec3 dir, const fishing_rod *rod) {
    fishing_cast_trace t;
fishing_cast_trace_arc(&t, w, origin, dir, rod, 1.0f / 60.0f);
return t.hit && t.in_water;
}

int fishing_cast_sample(world *w, vec3 origin, vec3 dir, const fishing_rod *rod,
                        vec3 *pts, int cap, float dt) {
    if (!pts || cap <= 0) return 0;
    if (dt <= 0.0f) dt = 1.0f / 30.0f;   // coarser steps for a preview is fine
    fishing_cast_trace t;
    int n;
    run_arc(&t, w, origin, dir, rod, dt, pts, cap, &n);
    return n;
}
