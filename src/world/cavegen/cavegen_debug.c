#include "cavegen_debug.h"

#include "../../config.h"
#include "../../util/log.h"
#include <stdio.h>

static char cell_glyph(uint8_t v) {
    switch (v) {
        case CAVEGEN_SOLID:   return '#';
        case CAVEGEN_AIR:     return ' ';
        case CAVEGEN_WORM:    return 'w';
        case CAVEGEN_FLOODED: return '~';
        case CAVEGEN_SEALED:  return 'x';
        default:              return '?';
    }
}

void cavegen_debug_slice_xz(const cavegen_grid *g, int y) {
    if (y < 0 || y >= CAVEGEN_DIM_Y) return;
    fprintf(stderr, "-- cavegen xz slice y=%d --\n", y);
    for (int z = 0; z < CAVEGEN_DIM_Z; z++) {
        char line[CAVEGEN_DIM_X + 1];
        for (int x = 0; x < CAVEGEN_DIM_X; x++)
            line[x] = cell_glyph(cavegen_grid_get(g, x, y, z));
        line[CAVEGEN_DIM_X] = '\0';
        fprintf(stderr, "%s\n", line);
    }
}

void cavegen_debug_slice_zy(const cavegen_grid *g, int x) {
    if (x < 0 || x >= CAVEGEN_DIM_X) return;
    fprintf(stderr, "-- cavegen zy slice x=%d (top-down) --\n", x);
    // print high y first so it reads the right way up.
    for (int y = CAVEGEN_DIM_Y - 1; y >= 0; y--) {
        char line[CAVEGEN_DIM_Z + 1];
        for (int z = 0; z < CAVEGEN_DIM_Z; z++)
            line[z] = cell_glyph(cavegen_grid_get(g, x, y, z));
        line[CAVEGEN_DIM_Z] = '\0';
        fprintf(stderr, "%3d %s\n", y, line);
    }
}

float cavegen_debug_open_ratio(const cavegen_grid *g) {
    int open = 0, total = 0;
    for (int i = 0; i < CAVEGEN_CELLS; i++) {
        uint8_t v = g->cells[i];
        total++;
        if (cavegen_cell_is_open(v)) open++;
    }
    return total ? (float)open / (float)total : 0.0f;
}

int cavegen_debug_validate(const cavegen_grid *g, const cavegen_params *p) {
    int problems = 0;

    // 1. nothing open below the floor, and nothing open into the surface crust.
    for (int z = 0; z < CAVEGEN_DIM_Z; z++) {
        for (int x = 0; x < CAVEGEN_DIM_X; x++) {
            int surf = cavegen_grid_height(g, x, z);
            for (int y = 0; y < CAVEGEN_DIM_Y; y++) {
                if (!cavegen_cell_is_open(cavegen_grid_get(g, x, y, z))) continue;
                if (y < p->min_y) {
                    LOGW("cavegen: open cell below floor at (%d,%d,%d)", x, y, z);
                    problems++;
                }
                if (y > surf - p->surface_margin) {
                    // worms are allowed to breach near surface, so only flag the
                    // automata-flavoured air, not worm/flooded.
                    if (cavegen_grid_get(g, x, y, z) == CAVEGEN_AIR) {
                        LOGW("cavegen: room breached crust at (%d,%d,%d) surf=%d",
                             x, y, z, surf);
                        problems++;
                    }
                }
            }
        }
    }

    // 2. a believable open ratio. way too much air means a runaway carve.
    float ratio = cavegen_debug_open_ratio(g);
    if (ratio > 0.45f) {
        LOGW("cavegen: open ratio %.1f%% looks runaway", ratio * 100.0f);
        problems++;
    }

    if (problems == 0)
        LOGD("cavegen: grid validated clean (open %.1f%%)", ratio * 100.0f);
    return problems;
}
