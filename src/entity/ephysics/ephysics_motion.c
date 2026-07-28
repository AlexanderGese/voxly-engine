#include "ephysics_motion.h"
#include <math.h>

vec3 ephysics_motion_ballistic(vec3 pos, vec3 vel, float gravity, float t) {
    // gravity passed as a positive magnitude, pulls -y.
    vec3 p = vec3_add(pos, vec3_scale(vel, t));
    p.y -= 0.5f * gravity * t * t;
    return p;
}

float ephysics_motion_solve_arc(vec3 from, vec3 to, float speed, float gravity,
                                vec3 *out_vel) {
    // standard projectile range solve. work in the vertical plane: horizontal
    // distance d and height difference h.
    vec3 flat = vec3_new(to.x - from.x, 0.0f, to.z - from.z);
    float d = vec3_length(flat);
    float h = to.y - from.y;

    float s2 = speed * speed;
    float g  = gravity;

    // discriminant of the launch-angle quadratic. negative -> unreachable.
    float disc = s2 * s2 - g * (g * d * d + 2.0f * h * s2);
    if (disc < 0.0f) return -1.0f;

    float root = sqrtf(disc);
    // low arc uses the minus root for the tangent of the launch angle.
    float tan_theta = (s2 - root) / (g * d + 1e-6f);
    float theta = atanf(tan_theta);

    // horizontal and vertical speed components.
    float vh = speed * cosf(theta);
    float vy = speed * sinf(theta);
    if (vh < 1e-4f) return -1.0f;

    float tof = d / vh;

    if (out_vel) {
        vec3 dir = (d > 1e-4f) ? vec3_scale(flat, 1.0f / d) : VEC3_FWD;
        *out_vel = vec3_new(dir.x * vh, vy, dir.z * vh);
    }
    return tof;
}

vec3 ephysics_motion_lead(vec3 shooter, vec3 target, vec3 tvel, float speed) {
    // fixed-point: guess time from current distance, advance the target, repeat.
    // converges fast for sane speeds; three passes is plenty.
    vec3 aim = target;
    for (int i = 0; i < 3; i++) {
        float dist = vec3_distance(shooter, aim);
        float t = (speed > 1e-4f) ? dist / speed : 0.0f;
        aim = vec3_add(target, vec3_scale(tvel, t));
    }
    return aim;
}

vec3 ephysics_motion_interp(vec3 prev, vec3 cur, float alpha) {
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;   // clamp: never extrapolate past `cur`
    return vec3_lerp(prev, cur, alpha);
}
