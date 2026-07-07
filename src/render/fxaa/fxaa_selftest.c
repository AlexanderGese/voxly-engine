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
y < GRID;
y++)
        for (int x = 0;
x < GRID;
x++)
            g->v[y][x] = v;
y < GRID;
y++)
        for (int x = 0;
