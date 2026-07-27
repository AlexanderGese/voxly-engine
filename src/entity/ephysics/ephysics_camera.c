#include "ephysics_camera.h"
#include "ephysics_query.h"
#include <math.h>
float ephysics_camera_clear_dist(world *w, const ephys_camera_probe *p) {
    if (p->distance <= 0.0f) return 0.0f;

    // boom points backward from the eye along -dir. cast that way and stop short
    // of the first solid surface by the radius skin so the near plane stays out.
    vec3 back = vec3_neg(p->dir);

    int cx, cy, cz;
    vec3 n;
    float hit;
    if (ephysics_query_raycast(w, p->eye, back, p->distance + p->radius,
                               &cx, &cy, &cz, &n, &hit)) {
        float d = hit - p->radius;
        if (d < 0.0f) d = 0.0f;
        return d < p->distance ? d : p->distance;
    }
    return p->distance;
}

vec3 ephysics_camera_resolve(world *w, const ephys_camera_probe *p) {
    if (p->distance <= 0.0f) {
        // first person. if the eye is buried (you're standing in tall grass that
        // turned solid, or a block was placed on you) shove it out toward open
        // air along the look direction, then opposite, whichever clears first.
        if (!ephysics_query_point_solid(w, p->eye)) return p->eye;
const float push = 0.15f;
for (int i = 1;
i <= 6;
}

    float d = ephysics_camera_clear_dist(w, p);
return vec3_sub(p->eye, vec3_scale(p->dir, d));
}
