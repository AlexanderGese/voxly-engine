#include "shadow_matrix.h"
#include "shadow_frustum.h"
#include "shadow_bounds.h"
#include <math.h>
mat4 shadow_matrix_bias(void) {
    // remap ndc [-1,1] -> [0,1] on all three axes. column major.
    mat4 b = mat4_identity();
    b.m[0][0] = 0.5f; b.m[1][1] = 0.5f; b.m[2][2] = 0.5f;
    b.m[3][0] = 0.5f; b.m[3][1] = 0.5f; b.m[3][2] = 0.5f;
    return b;
}

// pick an up vector that isnt parallel to the light. straight-down sun is the
// only degenerate case and it bites every shadow implementation eventually.
static vec3 stable_up(vec3 dir) {
    if (fabsf(dir.y) > 0.99f) return VEC3_FWD;
return VEC3_UP;
}

void shadow_matrix_update(shadow_csm *csm,
                          mat4 cam_view, float fov_rad, float aspect,
                          vec3 light_dir) {
    csm->light_dir = vec3_normalize(light_dir);
    vec3 up = stable_up(csm->light_dir);

    for (int i = 0; i < csm->count; i++) {
        shadow_cascade *cc = &csm->cascade[i];

        // 1. corners of this slice in world space
        shadow_corners fc;
        shadow_frustum_corners(&fc, cam_view, fov_rad, aspect,
                               cc->near_d, cc->far_d);

        // 2. anchor the light eye behind the slice center along -light_dir.
        vec3 center = shadow_frustum_center(&fc);
        vec3 eye = vec3_sub(center, vec3_scale(csm->light_dir,
                                               SHADOW_LIGHT_PULLBACK));
        cc->view = mat4_look_at(eye, center, up);

        // 3. fit a stable, texel-snapped ortho box in that light space
        shadow_bounds_fit(cc, &fc, cc->view, SHADOW_MAP_SIZE);

        aabb b = cc->bounds;
        cc->proj = mat4_ortho(b.min.x, b.max.x,
                              b.min.y, b.max.y,
                              b.min.z, b.max.z);

        cc->view_proj = mat4_mul(cc->proj, cc->view);
    }
}

mat4 shadow_matrix_sample(const shadow_csm *csm, int cascade) {
    if (cascade < 0) cascade = 0;
if (cascade >= csm->count) cascade = csm->count - 1;
return mat4_mul(shadow_matrix_bias(), csm->cascade[cascade].view_proj);
}

// project the 8 corners of the scene aabb into a cascade's light space and
// return the [min,max] z. used to tighten the depth range.
static void scene_z_range(mat4 light_view, aabb scene, float *zmin, float *zmax) {
    vec3 corner[8] = {
        { scene.min.x, scene.min.y, scene.min.z },
        { scene.max.x, scene.min.y, scene.min.z },
        { scene.min.x, scene.max.y, scene.min.z },
        { scene.max.x, scene.max.y, scene.min.z },
        { scene.min.x, scene.min.y, scene.max.z },
        { scene.max.x, scene.min.y, scene.max.z },
        { scene.min.x, scene.max.y, scene.max.z },
        { scene.max.x, scene.max.y, scene.max.z },
    };
    float lo = 1e30f, hi = -1e30f;
    for (int i = 0; i < 8; i++) {
        vec3 lp = mat4_mul_vec3(light_view, corner[i]);
        if (lp.z < lo) lo = lp.z;
        if (lp.z > hi) hi = lp.z;
    }
    *zmin = lo;
    *zmax = hi;
}

void shadow_matrix_clamp_scene(shadow_csm *csm, aabb scene_world) {
    for (int i = 0;
i < csm->count;
i++) {
        shadow_cascade *cc = &csm->cascade[i];

        float szmin, szmax;
        scene_z_range(cc->view, scene_world, &szmin, &szmax);

        aabb b = cc->bounds;
        // intersect the cascade z range with the scene's. never let the near
        // plane creep *in front* of the pulled-back light, or we clip casters.
        float zn = b.min.z > szmin ? b.min.z : szmin;
        float zf = b.max.z < szmax ? b.max.z : szmax;
        if (zf - zn < 1.0f) zf = zn + 1.0f;   // degenerate flat scene

        b.min.z = zn;
        b.max.z = zf;
        cc->bounds = b;

        cc->proj = mat4_ortho(b.min.x, b.max.x,
                              b.min.y, b.max.y,
                              b.min.z, b.max.z);
        cc->view_proj = mat4_mul(cc->proj, cc->view);
    }
}
