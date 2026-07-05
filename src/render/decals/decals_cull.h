#ifndef RENDER_DECALS_CULL_H
#define RENDER_DECALS_CULL_H

#include "decals_types.h"
#include "decals_pool.h"
#include "../../math/frustum.h"

// builds the per-frame visible list. walks the pool, drops decals that are
// fully faded, frustum-culls the projector aabb, distance-culls transient ones,
// then sorts the survivors so they layer deterministically (priority first,
// then phase so dying decals draw under fresh ones).

// an entry in the visible list. we keep the slot index plus a sort key so the
// sort doesnt have to dereference the pool repeatedly.
typedef struct {
    uint32_t slot;       // index into the pool
    uint32_t sort;       // packed key, ascending = draw earlier
    float    dist_sq;    // to the camera, for the distance fade in the shader
} decals_cull_item;

typedef struct {
    decals_cull_item items[DECALS_MAX];
    int   count;
    int   tested;        // how many live decals we looked at (stats)
    int   culled_frustum;
    int   culled_dist;
    int   culled_alpha;
} decals_cull_result;

// gather visible decals. `vp` is view*proj for the frustum, `cam` the camera
// world position for distance culling, `max_dist` the transient cutoff (decals
// past it that arent world-locked are skipped). result is fully overwritten.
void decals_cull_gather(decals_cull_result *out, const decals_pool *p,
                        mat4 vp, vec3 cam, float max_dist);

#endif
