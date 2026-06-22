#include "ravine_debug.h"

#include "../block.h"
#include "../../util/log.h"
#include <stdio.h>

static char kind_glyph(uint8_t k) {
    switch (k) {
        case RAVINE_OUTSIDE: return '.';
        case RAVINE_WALL:    return '#';
        case RAVINE_FLOOR:   return '_';
        case RAVINE_RIM:     return ':';
        default:             return '?';
    }
}

void ravine_debug_print_kinds(const ravine_field *f) {
    fprintf(stderr, "-- ravine kinds (cx=%d cz=%d) --\n",
            f->origin.chunk_cx, f->origin.chunk_cz);
    for (int z = 0; z < RAVINE_DIM_Z; z++) {
        char line[RAVINE_DIM_X + 1];
        for (int x = 0; x < RAVINE_DIM_X; x++) {
            int i = ravine_cell_index(x, z);
            line[x] = (i < 0) ? '?' : kind_glyph(f->kind[i]);
        }
        line[RAVINE_DIM_X] = '\0';
        fprintf(stderr, "%s\n", line);
    }
}

void ravine_debug_print_depth(const ravine_field *f) {
    static const char ramp[] = ".,-:;=+*#@";
    fprintf(stderr, "-- ravine depth (cx=%d cz=%d) --\n",
            f->origin.chunk_cx, f->origin.chunk_cz);
    for (int z = 0; z < RAVINE_DIM_Z; z++) {
        char line[RAVINE_DIM_X + 1];
        for (int x = 0; x < RAVINE_DIM_X; x++) {
            int i = ravine_cell_index(x, z);
            int cut = (i < 0) ? 0 : f->cut[i];
            int b = cut / 4;            // ~4 blocks per ramp step
            b = ravine_clampi(b, 0, 9);
            line[x] = ramp[b];
        }
        line[RAVINE_DIM_X] = '\0';
        fprintf(stderr, "%s\n", line);
    }
}

void ravine_debug_dump_path(const ravine_path *path) {
    fprintf(stderr, "-- ravine path: %d knots, reach=%.1f --\n",
            path->count, path->reach);
    for (int i = 0; i < path->count; i++) {
        const ravine_knot *k = &path->knots[i];
        fprintf(stderr, "  [%2d] x=%.1f z=%.1f hw=%.2f floor=%.1f\n",
                i, k->x, k->z, k->half_width, k->floor_y);
    }
}

void ravine_debug_dump_strata(const ravine_strata *s) {
    fprintf(stderr, "-- ravine strata: %d bands --\n", s->count);
    for (int i = 0; i < s->count; i++) {
        const ravine_band *b = &s->bands[i];
        const block_info *bi = block_get(b->id);
        fprintf(stderr, "  [%d] y %d..%d  %-7s hardness=%u\n",
                i, b->y_lo, b->y_hi, bi ? bi->name : "?", b->hardness);
    }
}

int ravine_debug_validate(const ravine_field *f, const ravine_params *p) {
    int problems = 0;
    for (int z = 0; z < RAVINE_DIM_Z; z++) {
        for (int x = 0; x < RAVINE_DIM_X; x++) {
            int idx = ravine_cell_index(x, z);
            if (idx < 0) continue;
            uint8_t k = f->kind[idx];
            if (k == RAVINE_OUTSIDE) {
                if (f->cut[idx] > 0) {
                    LOGE("ravine: outside cell (%d,%d) has cut %d",
                         x, z, f->cut[idx]);
                    problems++;
                }
                continue;
            }
            int surf  = f->surface[idx];
            int floor = f->floor_y[idx];

            if (floor >= surf) {
                LOGE("ravine: cell (%d,%d) floor %d >= surface %d",
                     x, z, floor, surf);
                problems++;
            }
            if (floor < p->min_floor_y) {
                LOGE("ravine: cell (%d,%d) floor %d under guard %d",
                     x, z, floor, p->min_floor_y);
                problems++;
            }
            if (f->cut[idx] > p->max_depth + 1) {
                LOGE("ravine: cell (%d,%d) cut %d past max_depth %d",
                     x, z, f->cut[idx], p->max_depth);
                problems++;
            }
        }
    }
    if (problems == 0)
        LOGD("ravine: field validated clean");
    return problems;
}
