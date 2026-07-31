#include "projectile_aim.h"
#include "projectile_ballistic.h"

#include <math.h>

vec3 projectile_aim_direct(vec3 from, vec3 to) {
    vec3 d = vec3_sub(to, from);
    float l = vec3_length(d);
    if (l < 1e-6f) return VEC3_FWD;     // degenerate, point somewhere sane
    return vec3_scale(d, 1.0f / l);
}

vec3 projectile_aim_lead(vec3 from, vec3 tpos, vec3 tvel, float speed) {
    if (speed < 1e-3f) return tpos;

    // fixed-point iteration on time-of-flight: guess the target stands still,
    // estimate flight time to its lead position, advance the target, repeat.
    // converges in a handful of steps for any sane closing speed.
    vec3 aim = tpos;
    for (int it = 0; it < 6; it++) {
        float dist = vec3_distance(from, aim);
        float tof  = dist / speed;
        vec3 next  = vec3_add(tpos, vec3_scale(tvel, tof));
        // bail early once the lead point stops moving.
        if (vec3_distance(next, aim) < 1e-3f) { aim = next; break; }
        aim = next;
    }
    return aim;
}

int projectile_aim_ballistic(projectile_kind kind, vec3 from, vec3 target,
                             float gravity, vec3 *out_vel) {
    const projectile_def *d = projectile_kind_def(kind);
    float v = d->speed;

    // split into a horizontal distance + a vertical delta, solve the launch
    // pitch in that plane, then re-expand into 3d along the horizontal heading.
    vec3 flat = vec3_sub(target, from);
    float dy = flat.y;
    flat.y = 0.0f;
    float dist = vec3_length(flat);

    if (dist < 1e-4f) {
        // straight up/down shot: no horizontal solve needed.
        if (out_vel) *out_vel = (vec3){ 0.0f, dy >= 0 ? v : -v, 0.0f };
        return 1;
    }

    vec3 fdir = vec3_scale(flat, 1.0f / dist);

    float pitch;
    if (!projectile_ballistic_solve_pitch(dist, dy, v, gravity, &pitch)) {
        return 0;   // out of range at this muzzle speed
    }

    // decompose muzzle speed into the horizontal heading and vertical climb.
    float vh = v * cosf(pitch);
    float vv = v * sinf(pitch);
    if (out_vel) {
        *out_vel = (vec3){ fdir.x * vh, vv, fdir.z * vh };
    }
    return 1;
}
