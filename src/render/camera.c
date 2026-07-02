#include "camera.h"

#include <math.h>

#define PI 3.14159265358979f

void camera_init(camera *c) {
    c->pos    = (vec3){0, 80, 0};
    c->yaw    = 0.0f;
    c->pitch  = 0.0f;
    c->fov    = 70.0f * PI / 180.0f;
    c->aspect = 16.0f / 9.0f;
    c->znear  = 0.1f;
    c->zfar   = 512.0f;
}

void camera_update(camera *c, float dx, float dy, float sensitivity) {
    c->yaw   += dx * sensitivity;
    c->pitch -= dy * sensitivity;

    float limit = 89.0f * PI / 180.0f;
    if (c->pitch >  limit) c->pitch =  limit;
    if (c->pitch < -limit) c->pitch = -limit;

    // wrap yaw, not strictly needed but keeps it small
    while (c->yaw >  2.0f * PI) c->yaw -= 2.0f * PI;
    while (c->yaw < -2.0f * PI) c->yaw += 2.0f * PI;
}

vec3 camera_forward(const camera *c) {
    return vec3_normalize((vec3){
        cosf(c->pitch) * sinf(c->yaw),
        sinf(c->pitch),
       -cosf(c->pitch) * cosf(c->yaw)
    });
}

vec3 camera_right(const camera *c) {
    return vec3_normalize(vec3_cross(camera_forward(c), VEC3_UP));
}

vec3 camera_up(const camera *c) {
    return vec3_cross(camera_right(c), camera_forward(c));
}

mat4 camera_view(const camera *c) {
    vec3 fwd = camera_forward(c);
    vec3 tgt = vec3_add(c->pos, fwd);
    return mat4_look_at(c->pos, tgt, VEC3_UP);
}

mat4 camera_proj(const camera *c) {
    return mat4_perspective(c->fov, c->aspect, c->znear, c->zfar);
}
