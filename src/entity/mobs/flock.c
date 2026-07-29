#include "flock.h"
#include "mob_util.h"

#include "../../math/vec3.h"
#include <math.h>

voxl_flock_params voxl_flock_default_params(void) {
    voxl_flock_params p;
    p.neighbor_radius   = 8.0f;
    p.separation_dist   = 2.0f;
    p.cohesion_weight   = 0.6f;
    p.alignment_weight  = 0.4f;
    p.separation_weight = 1.4f;
    p.max_speed         = 2.0f;
    return p;
}

vec3 voxl_flock_center(voxl_mob **mobs, int count) {
    vec3 c = VEC3_ZERO;
    if (count <= 0) return c;
    for (int i = 0; i < count; i++) {
        c = vec3_add(c, mobs[i]->pos);
    }
    return vec3_scale(c, 1.0f / (float)count);
}

void voxl_flock_step(voxl_mob **mobs, int count,
                     const voxl_flock_params *p, float dt) {
    if (count < 2 || !p) return;

    for (int i = 0; i < count; i++) {
        voxl_mob *me = mobs[i];

        vec3 coh = VEC3_ZERO;   // sum of neighbor positions
        vec3 sep = VEC3_ZERO;   // push away from very close neighbors
        vec3 ali = VEC3_ZERO;   // sum of neighbor headings
        int n = 0;

        for (int j = 0; j < count; j++) {
            if (j == i) continue;
            voxl_mob *other = mobs[j];
            float d = voxl_mob_flat_dist(me->pos, other->pos);
            if (d > p->neighbor_radius) continue;

            n++;
            coh = vec3_add(coh, other->pos);
            ali = vec3_add(ali, voxl_mob_heading(other->yaw));

            if (d < p->separation_dist && d > 0.0001f) {
                vec3 away = vec3_sub(me->pos, other->pos);
                away.y = 0.0f;
                // weight harder the closer they are.
                away = vec3_scale(away, (p->separation_dist - d) / d);
                sep = vec3_add(sep, away);
            }
        }

        if (n == 0) continue;

        vec3 steer = VEC3_ZERO;

        // cohesion: head toward the local center.
        vec3 center = vec3_scale(coh, 1.0f / (float)n);
        vec3 to_center = vec3_sub(center, me->pos);
        to_center.y = 0.0f;
        steer = vec3_add(steer, vec3_scale(to_center, p->cohesion_weight));

        // alignment: match average heading.
        vec3 avg_heading = vec3_scale(ali, 1.0f / (float)n);
        avg_heading.y = 0.0f;
        steer = vec3_add(steer, vec3_scale(avg_heading, p->alignment_weight));

        // separation.
        sep.y = 0.0f;
        steer = vec3_add(steer, vec3_scale(sep, p->separation_weight));

        // apply to velocity (xz), clamped.
        me->vel.x += steer.x * dt;
        me->vel.z += steer.z * dt;

        float spd = sqrtf(me->vel.x * me->vel.x + me->vel.z * me->vel.z);
        if (spd > p->max_speed && spd > 0.0001f) {
            float k = p->max_speed / spd;
            me->vel.x *= k;
            me->vel.z *= k;
        }
    }
}
