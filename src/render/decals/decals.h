#ifndef RENDER_DECALS_H
#define RENDER_DECALS_H
#include "decals_types.h"
#include "decals_pool.h"
#include "decals_atlas.h"
#include "decals_cull.h"
#include "decals_pass.h"
#include "../gl.h"
#include "../../math/mat4.h"
typedef struct {
    decals_pool        pool;
    decals_atlas       atlas;
    decals_cull_result vis;     // last gather, reused each frame
    decals_pass        pass;

    mat4  view_proj, inv_view_proj;
    vec3  cam_pos;
    float max_dist;             // transient-decal distance cutoff
    int   ready;                // 0 if gl init failed; render() then no-ops
} decals_system;
#endif
