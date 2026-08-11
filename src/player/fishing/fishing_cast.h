#ifndef PLAYER_FISHING_CAST_H
#define PLAYER_FISHING_CAST_H

#include "fishing_types.h"
#include "fishing_rod.h"
#include "../../math/vec3.h"
#include "../../world/world.h"

// the aim helper. before you let go of a cast we can trace the bobber's arc
// through the world and tell you where it would land, so a hud can draw a dotted
// preview and so the session can refuse a hopeless cast (straight into a wall).
// this is the same ballistic step the bobber sim uses, just run ahead of time.

#define FISHING_CAST_MAX_STEPS  128   // give up after this many sim steps

typedef struct {
    vec3  landing;        // where the bobber would come to rest
    int   hit;            // 1 if it reached water/ground, 0 if it ran off
    int   in_water;       // landed on water specifically
    int   steps;          // how many steps the trace took
    float distance;       // horizontal distance from origin to landing
} fishing_cast_trace;

// trace a cast from origin along dir at the rod's power. `dt` is the step size
// (use the frame dt or a fixed 1/60). fills out and returns out->hit.
int  fishing_cast_trace_arc(fishing_cast_trace *out, world *w,
                            vec3 origin, vec3 dir, const fishing_rod *rod, float dt);

// would this cast actually find water. convenience wrapper over the trace.
int  fishing_cast_is_viable(world *w, vec3 origin, vec3 dir, const fishing_rod *rod);

// sample the arc into an array of points for a preview line. writes up to `cap`
// points and returns how many it wrote.
int  fishing_cast_sample(world *w, vec3 origin, vec3 dir, const fishing_rod *rod,
                         vec3 *pts, int cap, float dt);

#endif
