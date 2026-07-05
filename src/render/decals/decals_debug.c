#include "decals_debug.h"
#include "decals_config.h"

#include <stdio.h>
#include <string.h>

void decals_debug_collect(const decals_system *s, decals_debug_stats *out) {
    memset(out, 0, sizeof *out);
    out->live    = decals_pool_live_count(&s->pool);
    out->visible = s->vis.count;
    out->drawn   = s->pass.drawn_last;
    out->culled_frustum = s->vis.culled_frustum;
    out->culled_dist    = s->vis.culled_dist;
    out->culled_alpha   = s->vis.culled_alpha;
    out->fill_ratio = (float)out->live / (float)DECALS_MAX;

    // tally by phase. dead slots are skipped, so phase_count[DEAD] stays 0.
    for (int i = 0; i < DECALS_MAX; i++) {
        const decals_decal *d = &s->pool.slots[i];
        if (!d->alive) continue;
        int p = (int)d->phase;
        if (p >= 0 && p < 4) out->phase_count[p]++;
    }
}

int decals_debug_format(const decals_debug_stats *st, char *buf, int cap) {
    if (cap <= 0) return 0;
    int n = snprintf(buf, (size_t)cap,
                     "decals live=%d vis=%d drawn=%d  cull[f=%d d=%d a=%d]  fill=%.0f%%",
                     st->live, st->visible, st->drawn,
                     st->culled_frustum, st->culled_dist, st->culled_alpha,
                     st->fill_ratio * 100.0f);
    if (n < 0) { buf[0] = '\0'; return 0; }
    if (n >= cap) n = cap - 1;
    buf[n] = '\0';
    return n;
}

// the eight corners of a projector box, in world space. order: the low/high bit
// of i picks -/+ along right/up/normal respectively.
static void box_corners(const decals_projector *p, vec3 c[8]) {
    for (int i = 0; i < 8; i++) {
        float sx = (i & 1) ? 1.0f : -1.0f;
        float sy = (i & 2) ? 1.0f : -1.0f;
        float sz = (i & 4) ? 1.0f : -1.0f;
        vec3 v = p->center;
        v = vec3_add(v, vec3_scale(p->right,  sx * p->half.x));
        v = vec3_add(v, vec3_scale(p->up,     sy * p->half.y));
        v = vec3_add(v, vec3_scale(p->normal, sz * p->half.z));
        c[i] = v;
    }
}

// the 12 edges as index pairs into the corner array above.
static const int k_edges[12][2] = {
    {0,1},{1,3},{3,2},{2,0},   // -z face loop
    {4,5},{5,7},{7,6},{6,4},   // +z face loop
    {0,4},{1,5},{2,6},{3,7},   // connecting struts
};

int decals_debug_box_lines(const decals_system *s, vec3 *out, int max_lines) {
    int written = 0;
    for (int i = 0; i < DECALS_MAX && written < max_lines; i++) {
        const decals_decal *d = &s->pool.slots[i];
        if (!d->alive) continue;

        vec3 c[8];
        box_corners(&d->proj, c);
        for (int e = 0; e < 12 && written < max_lines; e++) {
            out[written * 2 + 0] = c[k_edges[e][0]];
            out[written * 2 + 1] = c[k_edges[e][1]];
            written++;
        }
    }
    return written;
}
