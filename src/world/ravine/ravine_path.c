#include "ravine_path.h"

#include "ravine_rand.h"
#include "ravine_noise.h"
#include <math.h>
#include <stddef.h>

// --- construction -----------------------------------------------------------

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
}

void ravine_path_sample(const ravine_path *path, float t,
                        float *out_x, float *out_z,
                        float *out_hw, float *out_floor) {
    int n = path->count;
    if (n < 2) {
        if (out_x)     *out_x     = path->knots[0].x;
        if (out_z)     *out_z     = path->knots[0].z;
        if (out_hw)    *out_hw    = path->knots[0].half_width;
        if (out_floor) *out_floor = path->knots[0].floor_y;
        return;
    }
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    float ft = t * (float)(n - 1);
    int   seg = (int)ft;
    if (seg >= n - 1) seg = n - 2;
    float u = ft - (float)seg;

    int i0 = seg - 1; if (i0 < 0) i0 = 0;
    int i1 = seg;
    int i2 = seg + 1;
    int i3 = seg + 2; if (i3 > n - 1) i3 = n - 1;

    const ravine_knot *a = &path->knots[i0];
    const ravine_knot *b = &path->knots[i1];
    const ravine_knot *c = &path->knots[i2];
    const ravine_knot *d = &path->knots[i3];

    if (out_x)     *out_x     = cr(a->x, b->x, c->x, d->x, u);
    if (out_z)     *out_z     = cr(a->z, b->z, c->z, d->z, u);
    if (out_hw)    *out_hw     = cr(a->half_width, b->half_width,
                                    c->half_width, d->half_width, u);
    if (out_floor) *out_floor  = cr(a->floor_y, b->floor_y,
                                    c->floor_y, d->floor_y, u);
}

// --- nearest-point query ----------------------------------------------------

static float seg_dist(float px, float pz, float ax, float az,
                      float bx, float bz, float *t) {
    float dx = bx - ax, dz = bz - az;
    float len2 = dx * dx + dz * dz;
    float s;
    if (len2 < 1e-6f) {
        s = 0.0f;
    } else {
        s = ((px - ax) * dx + (pz - az) * dz) / len2;
        if (s < 0.0f) s = 0.0f;
        if (s > 1.0f) s = 1.0f;
    }
    float cx = ax + dx * s, cz = az + dz * s;
    float ex = px - cx, ez = pz - cz;
    *t = s;
    return sqrtf(ex * ex + ez * ez);
}

int ravine_path_nearest(const ravine_path *path, float cx, float cz,
                        float *out_dist, float *out_hw, float *out_floor) {
    int n = path->count;
    if (n < 2) return 0;

    float best = 1e30f;
    int   best_seg = 0;
    float best_t = 0.0f;

    // the control polygon tracks the spline closely at our knot spacing, so the
    // distance test against the straight segments is plenty accurate.
    for (int i = 0; i < n - 1; i++) {
        const ravine_knot *a = &path->knots[i];
        const ravine_knot *b = &path->knots[i + 1];
        float t;
        float d = seg_dist(cx, cz, a->x, a->z, b->x, b->z, &t);
        if (d < best) { best = d; best_seg = i; best_t = t; }
    }

    const ravine_knot *a = &path->knots[best_seg];
    const ravine_knot *b = &path->knots[best_seg + 1];
    if (out_dist)  *out_dist  = best;
    if (out_hw)    *out_hw    = a->half_width + (b->half_width - a->half_width) * best_t;
    if (out_floor) *out_floor = a->floor_y   + (b->floor_y   - a->floor_y)   * best_t;
    return 1;
}

int ravine_path_touches(const ravine_path *path,
                        float min_x, float min_z, float max_x, float max_z) {
    float lx = path->min_x - path->reach, hx = path->max_x + path->reach;
    float lz = path->min_z - path->reach, hz = path->max_z + path->reach;
    if (max_x < lx || min_x > hx) return 0;
    if (max_z < lz || min_z > hz) return 0;
    return 1;
}
