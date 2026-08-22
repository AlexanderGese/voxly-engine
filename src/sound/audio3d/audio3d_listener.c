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
vec3 u = vec3_normalize(up);
vec3 r = vec3_cross(f, u);
u = vec3_normalize(vec3_cross(r, f));
l->fwd   = f;
l->right = r;
l->up    = u;
if (gain < 0.0f) gain = 0.0f;
if (gain > 1.0f) gain = 1.0f;
l->gain = gain;
