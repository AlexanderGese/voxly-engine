#include "shadow_debug.h"
#include "shadow_frustum.h"
#include "../../util/log.h"

vec3 shadow_debug_cascade_color(int cascade) {
    // red / green / blue / yellow. wraps if someone bumps the count past 4.
    static const vec3 palette[4] = {
        { 1.0f, 0.3f, 0.3f },
        { 0.3f, 1.0f, 0.3f },
        { 0.3f, 0.5f, 1.0f },
        { 1.0f, 0.9f, 0.3f },
    };
    if (cascade < 0) cascade = 0;
    return palette[cascade & 3];
}

void shadow_debug_dump(const shadow_csm *csm) {
    LOGI("shadow: light_dir (%.2f %.2f %.2f) enabled=%d",
         (double)csm->light_dir.x, (double)csm->light_dir.y,
         (double)csm->light_dir.z, csm->enabled);
    for (int i = 0; i < csm->count; i++) {
        const shadow_cascade *c = &csm->cascade[i];
        aabb b = c->bounds;
        float w = b.max.x - b.min.x;
        float h = b.max.y - b.min.y;
        float d = b.max.z - b.min.z;
        LOGI("  cascade %d: split %.1f..%.1f  box %.1fx%.1fx%.1f  texel %.3f",
             i, (double)c->near_d, (double)c->far_d,
             (double)w, (double)h, (double)d, (double)c->texel_world);
    }
}

// indices into the 8 corners for the 12 edges of a frustum box.
static const int EDGE[12][2] = {
    {0,1},{1,2},{2,3},{3,0},   // near quad
    {4,5},{5,6},{6,7},{7,4},   // far quad
    {0,4},{1,5},{2,6},{3,7},   // connecting struts
};

int shadow_debug_slice_lines(const shadow_csm *csm, int cascade,
                             mat4 cam_view, float fov_rad, float aspect,
                             vec3 *out) {
    if (cascade < 0) cascade = 0;
    if (cascade >= csm->count) cascade = csm->count - 1;

    shadow_corners fc;
    shadow_frustum_corners(&fc, cam_view, fov_rad, aspect,
                           csm->cascade[cascade].near_d,
                           csm->cascade[cascade].far_d);

    int n = 0;
    for (int e = 0; e < 12; e++) {
        out[n++] = fc.c[EDGE[e][0]];
        out[n++] = fc.c[EDGE[e][1]];
    }
    return n;  // 24
}

float shadow_debug_texel_density(const shadow_csm *csm, int cascade) {
    if (cascade < 0) cascade = 0;
    if (cascade >= csm->count) cascade = csm->count - 1;
    float t = csm->cascade[cascade].texel_world;
    return t > 0.0f ? 1.0f / t : 0.0f;
}
