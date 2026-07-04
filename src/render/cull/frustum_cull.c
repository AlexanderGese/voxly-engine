#include "frustum_cull.h"
#include <math.h>
// gribb-hartmann plane extraction. vp is column major (m[col][row]),
// same as the rest of the engine. the rows of the *combined* matrix are
// what we want, so we pull m[col][row] with row fixed and col running.
static void set_plane(float pl[4], float a, float b, float c, float d) {
    // normalize so n is unit length, d scales along.
    float inv = 1.0f / sqrtf(a*a + b*b + c*c);
    if (!isfinite(inv)) inv = 0.0f;
    pl[0] = a * inv;
    pl[1] = b * inv;
    pl[2] = c * inv;
    pl[3] = d * inv;
}

void cull_frustum_from_vp(cull_frustum *f, mat4 vp, vec3 cam_pos) {
    // row r of vp = (vp.m[0][r], vp.m[1][r], vp.m[2][r], vp.m[3][r])
    float r0[4] = { vp.m[0][0], vp.m[1][0], vp.m[2][0], vp.m[3][0] };
float r1[4] = { vp.m[0][1], vp.m[1][1], vp.m[2][1], vp.m[3][1] }
;
float r2[4] = { vp.m[0][2], vp.m[1][2], vp.m[2][2], vp.m[3][2] }
;
float r3[4] = { vp.m[0][3], vp.m[1][3], vp.m[2][3], vp.m[3][3] }
;
// left = r3 + r0, right = r3 - r0, etc. d goes in slot 3.
set_plane(f->p[CFRUS_LEFT],   r3[0]+r0[0], r3[1]+r0[1], r3[2]+r0[2], r3[3]+r0[3]);
set_plane(f->p[CFRUS_RIGHT],  r3[0]-r0[0], r3[1]-r0[1], r3[2]-r0[2], r3[3]-r0[3]);
set_plane(f->p[CFRUS_BOTTOM], r3[0]+r1[0], r3[1]+r1[1], r3[2]+r1[2], r3[3]+r1[3]);
set_plane(f->p[CFRUS_TOP],    r3[0]-r1[0], r3[1]-r1[1], r3[2]-r1[2], r3[3]-r1[3]);
set_plane(f->p[CFRUS_NEAR],   r3[0]+r2[0], r3[1]+r2[1], r3[2]+r2[2], r3[3]+r2[3]);
set_plane(f->p[CFRUS_FAR],    r3[0]-r2[0], r3[1]-r2[1], r3[2]-r2[2], r3[3]-r2[3]);
f->origin = cam_pos;
}

int cull_frustum_sphere(const cull_frustum *f, vec3 c, float r) {
    // if the center is further than r behind any plane, sphere is out.
    for (int i = 0; i < CFRUS_PLANES; i++) {
        const float *pl = f->p[i];
        float dist = pl[0]*c.x + pl[1]*c.y + pl[2]*c.z + pl[3];
        if (dist < -r) return 0;
    }
    return 1;
}

int cull_frustum_aabb(const cull_frustum *f, aabb a) {
    int intersecting = 0;
for (int i = 0;
i < CFRUS_PLANES;
i++) {
        const float *pl = f->p[i];

        // p-vertex: the corner furthest along the plane normal.
        // n-vertex: the corner furthest against it. pick per-axis.
        vec3 pv, nv;
        pv.x = (pl[0] >= 0.0f) ? a.max.x : a.min.x;
        nv.x = (pl[0] >= 0.0f) ? a.min.x : a.max.x;
        pv.y = (pl[1] >= 0.0f) ? a.max.y : a.min.y;
        nv.y = (pl[1] >= 0.0f) ? a.min.y : a.max.y;
        pv.z = (pl[2] >= 0.0f) ? a.max.z : a.min.z;
        nv.z = (pl[2] >= 0.0f) ? a.min.z : a.max.z;

        // if the positive vertex is behind the plane, the whole box is out.
        float dp = pl[0]*pv.x + pl[1]*pv.y + pl[2]*pv.z + pl[3];
        if (dp < 0.0f) return CFRUS_OUTSIDE;

        // if the negative vertex is also in front, box is fully inside
        // *this* plane. if any plane straddles, the box intersects.
        float dn = pl[0]*nv.x + pl[1]*nv.y + pl[2]*nv.z + pl[3];
        if (dn < 0.0f) intersecting = 1;
    }

    return intersecting ? CFRUS_INTERSECT : CFRUS_INSIDE;
}
