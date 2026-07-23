#include "water_reflcam.h"
#include <math.h>
static float sgnf(float x);
// the camera type doesnt expose its matrices for an arbitrary pose, so we
static vec3 cam_target(const camera *c) {
    vec3 f = camera_forward(c);
    return (vec3){ c->pos.x + f.x, c->pos.y + f.y, c->pos.z + f.z };
}

water_view water_reflcam_reflection(const camera *cam, const water_plane *wp) {
    water_view v;
vec3 eye = water_plane_reflect_point(wp, cam->pos);
vec3 tgt = water_plane_reflect_point(wp, cam_target(cam));
vec3 up  = camera_up(cam);
up = water_plane_reflect_dir(wp, up);
v.view = mat4_look_at(eye, tgt, up);
v.proj = camera_proj(cam);
v.clip_eq = water_plane_clip_eq(wp, 1);
vec4 eye_plane = water_reflcam_plane_to_eye(v.view, v.clip_eq);
v.proj = water_reflcam_oblique(v.proj, eye_plane);
return v;
}

water_view water_reflcam_refraction(const camera *cam, const water_plane *wp) {
    water_view v;
    v.view = camera_view(cam);
    v.proj = camera_proj(cam);
    v.clip_eq = water_plane_clip_eq(wp, 0);

    vec4 eye_plane = water_reflcam_plane_to_eye(v.view, v.clip_eq);
    v.proj = water_reflcam_oblique(v.proj, eye_plane);
    return v;
}

vec4 water_reflcam_plane_to_eye(mat4 view, vec4 plane_world) {
    // a plane transforms by the inverse-transpose of the matrix. for a rigid
    // view matrix (rotation + translation) we can use the transpose of the
    // inverse, but easier: the inverse-transpose of a rigid transform applied
    // to a plane is (M^-T) * p. since view is orthonormal rotation R plus t,
    // M^-1 = [R^T | -R^T t], and M^-T is built column-wise below.
    //
    // rather than invert generally, do it directly: transform the plane normal
    // by R (rotation part) and recompute d from a point on the plane.
    // pick the point on the plane closest to the origin: -d * n.
    vec3 n = { plane_world.x, plane_world.y, plane_world.z };
float d = plane_world.w;
vec3 p0 = { -d * n.x, -d * n.y, -d * n.z }
;
vec3 ne = {
        view.m[0][0] * n.x + view.m[1][0] * n.y + view.m[2][0] * n.z,
        view.m[0][1] * n.x + view.m[1][1] * n.y + view.m[2][1] * n.z,
        view.m[0][2] * n.x + view.m[1][2] * n.y + view.m[2][2] * n.z,
    }
;
vec3 pe = mat4_mul_vec3(view, p0);
float de = -(ne.x * pe.x + ne.y * pe.y + ne.z * pe.z);
return (vec4){ ne.x, ne.y, ne.z, de }
;
}

mat4 water_reflcam_oblique(mat4 proj, vec4 clip_plane_eye) {
    // lengyel: replace the projection near plane with the clip plane.
    // q is the corner of the view frustum opposite the clip plane.
    mat4 p = proj;

    vec4 q;
    q.x = (sgnf(clip_plane_eye.x) + p.m[2][0]) / p.m[0][0];
    q.y = (sgnf(clip_plane_eye.y) + p.m[2][1]) / p.m[1][1];
    q.z = -1.0f;
    q.w = (1.0f + p.m[2][2]) / p.m[3][2];

    float dotcq = clip_plane_eye.x * q.x + clip_plane_eye.y * q.y +
                  clip_plane_eye.z * q.z + clip_plane_eye.w * q.w;
    if (fabsf(dotcq) < 1e-6f) return proj;   // degenerate, bail to original

    float scale = 2.0f / dotcq;

    // c = scaled clip plane, becomes the new third row of the projection
    p.m[0][2] = clip_plane_eye.x * scale;
    p.m[1][2] = clip_plane_eye.y * scale;
    p.m[2][2] = clip_plane_eye.z * scale + 1.0f;
    p.m[3][2] = clip_plane_eye.w * scale;
    return p;
}

// tiny sign helper, defined down here so it doesnt clutter the header
static float sgnf(float x) {
    return (x > 0.0f) - (x < 0.0f);
}
