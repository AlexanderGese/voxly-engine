#include "fxaa_selftest.h"
#include "fxaa_edge.h"
#include "fxaa_quality.h"
#include "fxaa_params.h"
#include "fxaa_settings.h"
#include "fxaa_history.h"
#include "../../util/log.h"

#include <math.h>

// a small luma grid the edge callback samples from. origin is fixed at the
// center so dx/dy offsets map straight into the grid.
#define GRID 9
#define GRID_C (GRID / 2)

typedef struct {
    float v[GRID][GRID];
} luma_grid;

// callback: clamp out-of-range fetches to the edge (matches CLAMP_TO_EDGE).
static float grid_sample(int dx, int dy, void *user) {
    const luma_grid *g = (const luma_grid*)user;
    int x = GRID_C + dx;
    int y = GRID_C + dy;
    if (x < 0) x = 0;
    if (x >= GRID) x = GRID - 1;
    if (y < 0) y = 0;
    if (y >= GRID) y = GRID - 1;
    return g->v[y][x];
}

static void grid_fill(luma_grid *g, float v) {
    for (int y = 0; y < GRID; y++)
        for (int x = 0; x < GRID; x++)
            g->v[y][x] = v;
}

// vertical edge: left half lo, right half hi. (the boundary runs vertically.)
static void grid_vstep(luma_grid *g, float lo, float hi) {
    for (int y = 0; y < GRID; y++)
        for (int x = 0; x < GRID; x++)
            g->v[y][x] = (x < GRID_C) ? lo : hi;
}

// horizontal edge: top half lo, bottom half hi.
static void grid_hstep(luma_grid *g, float lo, float hi) {
    for (int y = 0; y < GRID; y++)
        for (int x = 0; x < GRID; x++)
            g->v[y][x] = (y < GRID_C) ? lo : hi;
}

static int approx(float a, float b, float eps) {
    float d = a - b;
    return (d < 0 ? -d : d) <= eps;
}

int fxaa_selftest_flat_no_edge(void) {
    fxaa_params p; fxaa_params_default(&p); fxaa_params_sanitize(&p);
    luma_grid g; grid_fill(&g, 0.5f);

    fxaa_edge_info info;
    int e = fxaa_edge_detect(&p, grid_sample, &g, &info);
    if (e || info.is_edge) {
        LOGE("fxaa selftest: flat field triggered an edge (contrast=%f)",
             info.contrast);
        return 0;
    }
    return 1;
}

int fxaa_selftest_vertical_edge(void) {
    fxaa_params p; fxaa_params_default(&p); fxaa_params_sanitize(&p);
    luma_grid g; grid_vstep(&g, 0.1f, 0.9f);

    fxaa_edge_info info;
    if (!fxaa_edge_detect(&p, grid_sample, &g, &info)) {
        LOGE("fxaa selftest: vertical step not detected");
        return 0;
    }
    // a boundary that runs vertically has its gradient along x, so the edge
    // orientation should report vertical (horizontal == 0).
    if (info.horizontal) {
        LOGE("fxaa selftest: vertical step misclassified as horizontal");
        return 0;
    }
    return 1;
}

int fxaa_selftest_horizontal_edge(void) {
    fxaa_params p; fxaa_params_default(&p); fxaa_params_sanitize(&p);
    luma_grid g; grid_hstep(&g, 0.1f, 0.9f);

    fxaa_edge_info info;
    if (!fxaa_edge_detect(&p, grid_sample, &g, &info)) {
        LOGE("fxaa selftest: horizontal step not detected");
        return 0;
    }
    if (!info.horizontal) {
        LOGE("fxaa selftest: horizontal step misclassified as vertical");
        return 0;
    }
    return 1;
}

int fxaa_selftest_dark_floor(void) {
    fxaa_params p; fxaa_params_default(&p);
    // crank the absolute floor up so a tiny near-black delta cant trigger.
    p.edge_threshold_min = 0.2f;
    fxaa_params_sanitize(&p);

    // relative contrast is enormous (0 vs 0.05 -> 100% relative) but the
    // absolute delta is below the floor.
    luma_grid g; grid_vstep(&g, 0.0f, 0.05f);

    fxaa_edge_info info;
    int e = fxaa_edge_detect(&p, grid_sample, &g, &info);
    if (e) {
        LOGE("fxaa selftest: dark floor failed to suppress edge (c=%f)",
             info.contrast);
        return 0;
    }
    return 1;
}

int fxaa_selftest_params_derive(void) {
    fxaa_params p; fxaa_params_default(&p);
    p.edge_threshold = 0.25f;
    p.subpix = 0.8f;
    fxaa_params_sanitize(&p);

    float d[4];
    fxaa_params_derive(&p, d);
    if (!approx(d[0], 0.8f, 1e-5f)) {
        LOGE("fxaa selftest: subpix passthrough wrong %f", d[0]);
        return 0;
    }
    if (!approx(d[1], 0.8f * 0.8f * 0.5f, 1e-5f)) {
        LOGE("fxaa selftest: subpix quad wrong %f", d[1]);
        return 0;
    }
    if (!approx(d[2], 1.0f / 0.25f, 1e-4f)) {
        LOGE("fxaa selftest: edge reciprocal wrong %f", d[2]);
        return 0;
    }

    // sanitize must drag a wild quality index back into the table.
    fxaa_params q; fxaa_params_default(&q);
    q.quality = 99;
    fxaa_params_sanitize(&q);
    if (q.quality < 0 || q.quality >= FXAA_QUALITY_COUNT) {
        LOGE("fxaa selftest: quality not clamped (%d)", q.quality);
        return 0;
    }
    return 1;
}

int fxaa_selftest_quality_reach(void) {
    float prev = -1.0f;
    for (int i = 0; i < FXAA_QUALITY_COUNT; i++) {
        const fxaa_quality *q = fxaa_quality_get(i);
        float r = fxaa_quality_reach(q);
        if (r < prev - 1e-4f) {
            LOGE("fxaa selftest: reach not monotonic at preset %d (%f < %f)",
                 i, r, prev);
            return 0;
        }
        prev = r;
    }
    return 1;
}

int fxaa_selftest_search_bounded(void) {
    fxaa_params p; fxaa_params_default(&p); fxaa_params_sanitize(&p);
    const fxaa_quality *q = fxaa_quality_get(p.quality);

    luma_grid g; grid_hstep(&g, 0.1f, 0.9f);
    fxaa_edge_info info;
    if (!fxaa_edge_detect(&p, grid_sample, &g, &info)) {
        LOGE("fxaa selftest: search setup failed to find edge");
        return 0;
    }
    float off = fxaa_edge_search(&p, q, grid_sample, &g, &info);
    if (off < -0.5f - 1e-4f || off > 0.5f + 1e-4f || isnan(off)) {
        LOGE("fxaa selftest: search offset out of range %f", off);
        return 0;
    }
    return 1;
}

int fxaa_selftest_settings_roundtrip(void) {
    fxaa_params p;
    fxaa_settings_apply(&p, FXAA_SET_OFF);
    if (p.enabled) {
        LOGE("fxaa selftest: OFF preset left fxaa enabled");
        return 0;
    }

    // apply a known preset, then classify should hand the same one back.
    fxaa_settings_apply(&p, FXAA_SET_QUALITY);
    fxaa_setting got = fxaa_settings_classify(&p);
    if (got != FXAA_SET_QUALITY) {
        LOGE("fxaa selftest: classify round-trip got %s, want quality",
             fxaa_settings_name(got));
        return 0;
    }
    return 1;
}

int fxaa_selftest_history_avg(void) {
    fxaa_history h;
    fxaa_history_reset(&h);

    // push a known ramp; the ring holds the last FXAA_HISTORY_LEN of them.
    float brute_sum = 0.0f;
    int kept = 0;
    for (int i = 0; i < FXAA_HISTORY_LEN + 7; i++) {
        float v = (float)(i % 10) / 10.0f;
        fxaa_history_push(&h, v);
    }
    // recompute the expected mean from the same tail the ring kept.
    for (int i = 7; i < FXAA_HISTORY_LEN + 7; i++) {
        brute_sum += (float)(i % 10) / 10.0f;
        kept++;
    }
    float want = brute_sum / (float)kept;
    float got = fxaa_history_avg(&h);
    if (!approx(got, want, 1e-4f)) {
        LOGE("fxaa selftest: history avg %f != brute %f", got, want);
        return 0;
    }
    if (got < 0.0f || got > 1.0f) {
        LOGE("fxaa selftest: history avg out of [0,1]: %f", got);
        return 0;
    }
    return 1;
}

int fxaa_selftest_run_all(void) {
    int fails = 0;
    fails += !fxaa_selftest_flat_no_edge();
    fails += !fxaa_selftest_vertical_edge();
    fails += !fxaa_selftest_horizontal_edge();
    fails += !fxaa_selftest_dark_floor();
    fails += !fxaa_selftest_params_derive();
    fails += !fxaa_selftest_quality_reach();
    fails += !fxaa_selftest_search_bounded();
    fails += !fxaa_selftest_settings_roundtrip();
    fails += !fxaa_selftest_history_avg();

    if (fails == 0) LOGI("fxaa selftest: all passed");
    else            LOGW("fxaa selftest: %d failure(s)", fails);
    return fails;
}
