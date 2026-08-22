#include "audio3d_listener.h"

#include <math.h>

void audio3d_listener_init(audio3d_listener *l) {
    if (!l) return;
    l->pos   = VEC3_ZERO;
    l->vel   = VEC3_ZERO;
    l->right = VEC3_RIGHT;
    l->up    = VEC3_UP;
    l->fwd   = VEC3_FWD;
    l->gain  = 1.0f;
}

void audio3d_listener_set_orientation(audio3d_listener *l, vec3 fwd, vec3 up) {
    if (!l) return;
    vec3 f = vec3_normalize(fwd);
    if (vec3_length_sq(f) < AUDIO3D_EPS) f = VEC3_FWD;

    // right = fwd x up. if they're parallel (looking along up) nudge up so the
    // cross product doesnt collapse to zero.
    vec3 u = vec3_normalize(up);
    vec3 r = vec3_cross(f, u);
    if (vec3_length_sq(r) < 1e-4f) {
        u = VEC3_FWD;                 // any non-parallel axis will do
        r = vec3_cross(f, u);
    }
    r = vec3_normalize(r);
    // re-derive a clean up so the basis is genuinely orthonormal.
    u = vec3_normalize(vec3_cross(r, f));

    l->fwd   = f;
    l->right = r;
    l->up    = u;
}

void audio3d_listener_set_angles(audio3d_listener *l, float yaw, float pitch) {
    if (!l) return;
    // standard fps spherical->cartesian. matches player/camera.c.
    float cp = cosf(pitch);
    vec3 fwd = {
        cosf(yaw) * cp,
        sinf(pitch),
        sinf(yaw) * cp
    };
    audio3d_listener_set_orientation(l, fwd, VEC3_UP);
}

void audio3d_listener_set_pos(audio3d_listener *l, vec3 pos) {
    if (l) l->pos = pos;
}

void audio3d_listener_move(audio3d_listener *l, vec3 pos, float dt) {
    if (!l) return;
    if (dt > AUDIO3D_EPS) {
        l->vel = vec3_scale(vec3_sub(pos, l->pos), 1.0f / dt);
    } else {
        l->vel = VEC3_ZERO;
    }
    l->pos = pos;
}

void audio3d_listener_set_gain(audio3d_listener *l, float gain) {
    if (!l) return;
    if (gain < 0.0f) gain = 0.0f;
    if (gain > 1.0f) gain = 1.0f;
    l->gain = gain;
}

vec3 audio3d_listener_local(const audio3d_listener *l, vec3 world) {
    vec3 d = vec3_sub(world, l->pos);
    return (vec3){
        vec3_dot(d, l->right),
        vec3_dot(d, l->up),
        vec3_dot(d, l->fwd)
    };
}
