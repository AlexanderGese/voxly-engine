#ifndef RENDER_CAMERA_H
#define RENDER_CAMERA_H

#include "../math/vec3.h"
#include "../math/mat4.h"

// first person camera. yaw=0 looks along -z, pitch=0 horizontal.

typedef struct {
    vec3  pos;
    float yaw;
    float pitch;
    float fov;          // radians
    float aspect;
    float znear;
    float zfar;
} camera;

void camera_init(camera *c);
void camera_update(camera *c, float dx, float dy, float sensitivity);

vec3 camera_forward(const camera *c);
vec3 camera_right(const camera *c);
vec3 camera_up(const camera *c);

mat4 camera_view(const camera *c);
mat4 camera_proj(const camera *c);

#endif
