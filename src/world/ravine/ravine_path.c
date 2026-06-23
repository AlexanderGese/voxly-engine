#include "ravine_path.h"
#include "ravine_rand.h"
#include "ravine_noise.h"
#include <math.h>
#include <stddef.h>
void ravine_path_build(ravine_path *path, const ravine_params *p,
                       float ax, float az, int floor_top, uint32_t stream) {
    ravine_rng rng;
    ravine_rng_seed(&rng, stream);

    int n = RAVINE_MAX_KNOTS;
    path->count = n;

    // initial heading + a steady drift bias so the canyon arcs one way rather
    // than wandering back over itself.
    float heading = ravine_rng_frange(&rng, 0.0f, 6.2831853f);
    float drift = ravine_rng_frange(&rng, -p->drift, p->drift) * 0.1f;

    float x = ax, z = az;
    float floor = (float)floor_top;

    for (int i = 0; i < n; i++) {
        ravine_knot *k = &path->knots[i];

        // width breathes around half_width via the wobble knob. clamp so it
        // never inverts into a negative-width canyon (ask me how i know).
        float w = ravine_rng_frange(&rng, 1.0f - p->width_wobble,
                                          1.0f + p->width_wobble);
        k->half_width = p->half_width * w;
        if (k->half_width < 1.5f) k->half_width = 1.5f;

        k->floor_y = floor;
        k->x = x;
        k->z = z;

        // advance one span. heading wanders inside knot_jitter, nudged by a slow
        // coherent noise field plus the constant drift.
        float nz = ravine_value2(x * 0.06f, z * 0.06f, p->seed ^ 0x51a7u);
        float wander = ravine_rng_frange(&rng, -1.0f, 1.0f)
                       * (p->knot_jitter * 0.05f);
        heading += wander + nz * 0.12f + drift;

        x += cosf(heading) * (float)p->knot_span;
        z += sinf(heading) * (float)p->knot_span;
        // gentle descent so the floor trends downhill end to end.
        floor -= 0.6f;
    }

    // cache cell-space bounds + reach for the quick reject.
    path->min_x = path->max_x = path->knots[0].x;
    path->min_z = path->max_z = path->knots[0].z;
    path->reach = 0.0f;
    for (int i = 0; i < n; i++) {
        ravine_knot *k = &path->knots[i];
        if (k->x < path->min_x) path->min_x = k->x;
        if (k->x > path->max_x) path->max_x = k->x;
        if (k->z < path->min_z) path->min_z = k->z;
        if (k->z > path->max_z) path->max_z = k->z;
        // reach has to cover the floor half-width plus the wall run-out so the
        // mask's bank columns still fall inside the bounds test.
        float r = k->half_width + (float)p->max_depth * p->wall_slope + 2.0f;
        if (r > path->reach) path->reach = r;
    }
}

// --- catmull-rom sampling ---------------------------------------------------

static float cr(float p0, float p1, float p2, float p3, float t) {
    float t2 = t * t, t3 = t2 * t;
return 0.5f * ((2.0f * p1) +
                   (-p0 + p2) * t +
                   (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
                   (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
float len2 = dx * dx + dz * dz;
float s;
if (s < 0.0f) s = 0.0f;
if (s > 1.0f) s = 1.0f;
}
    float cx = ax + dx * s, cz = az + dz * s;
float ex = px - cx, ez = pz - cz;
*t = s;
return sqrtf(ex * ex + ez * ez);
float lz = path->min_z - path->reach, hz = path->max_z + path->reach;
if (max_x < lx || min_x > hx) return 0;
if (max_z < lz || min_z > hz) return 0;
return 1;
}
