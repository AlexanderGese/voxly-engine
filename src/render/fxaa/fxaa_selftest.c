#include "fxaa_selftest.h"
#include "fxaa_edge.h"
#include "fxaa_quality.h"
#include "fxaa_params.h"
#include "fxaa_settings.h"
#include "fxaa_history.h"
#include "../../util/log.h"
#include <math.h>
#define GRID 9
#define GRID_C (GRID / 2)
typedef struct {
    float v[GRID][GRID];
} luma_grid;
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
    for (int y = 0;
y < GRID;
y++)
        for (int x = 0;
x < GRID;
x++)
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
    for (int y = 0;
y < GRID;
y++)
        for (int x = 0;
x < GRID;
x++)
            g->v[y][x] = (y < GRID_C) ? lo : hi;
}

static int approx(float a, float b, float eps) {
    float d = a - b;
    return (d < 0 ? -d : d) <= eps;
}

int fxaa_selftest_flat_no_edge(void) {
    fxaa_params p;
fxaa_params_default(&p);
fxaa_params_sanitize(&p);
luma_grid g;
grid_fill(&g, 0.5f);
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
    fxaa_params p;
fxaa_params_default(&p);
fxaa_params_sanitize(&p);
luma_grid g;
grid_hstep(&g, 0.1f, 0.9f);
fxaa_edge_info info;
return 0;
}
    return 1;
fxaa_params_default(&p);
p.edge_threshold = 0.25f;
p.subpix = 0.8f;
fxaa_params_sanitize(&p);
float d[4];
fxaa_params_derive(&p, d);
return 0;
fxaa_params_default(&q);
q.quality = 99;
fxaa_params_sanitize(&q);
fxaa_params_default(&p);
fxaa_params_sanitize(&p);
const fxaa_quality *q = fxaa_quality_get(p.quality);
luma_grid g;
grid_hstep(&g, 0.1f, 0.9f);
fxaa_edge_info info;
fxaa_history_reset(&h);
float brute_sum = 0.0f;
int kept = 0;
for (int i = 0;
i < FXAA_HISTORY_LEN + 7;
i < FXAA_HISTORY_LEN + 7;
float got = fxaa_history_avg(&h);
return 0;
}
    return 1;
