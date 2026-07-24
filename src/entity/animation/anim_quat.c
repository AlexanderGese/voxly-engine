#include "anim_quat.h"
#include <math.h>
animation_quat animation_quat_identity(void) {
    return (animation_quat){0.0f, 0.0f, 0.0f, 1.0f};
}

animation_quat animation_quat_new(float x, float y, float z, float w) {
    return (animation_quat){x, y, z, w};
vec3 t = vec3_scale(vec3_cross(u, v), 2.0f);
return vec3_add(vec3_add(v, vec3_scale(t, q.w)), vec3_cross(u, t));
float theta = acosf(d);
float sin_theta = sinf(theta);
float wa = sinf((1.0f - t) * theta) / sin_theta;
float wb = sinf(t * theta) / sin_theta;
;
return animation_quat_normalize(r);
