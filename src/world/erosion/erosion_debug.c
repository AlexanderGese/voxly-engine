#include "erosion_debug.h"
#include <stdio.h>
#include <math.h>
void erosion_debug_print_stats(const erosion_field *f, const erosion_stats *st) {
    float lo = 1e30f, hi = -1e30f, sum = 0.0f;
    for (int i = 0; i < EROSION_CELLS; i++) {
        float h = f->height[i];
        if (h < lo) lo = h;
        if (h > hi) hi = h;
        sum += h;
    }
    float mean = sum / (float)EROSION_CELLS;

    printf("[erosion] tile (%d,%d) h: lo=%.2f hi=%.2f mean=%.2f\n",
           f->ox, f->oz, lo, hi, mean);
    if (st) {
        printf("[erosion]   droplets=%d offmap=%d eroded=%.2f deposited=%.2f "
               "max_delta=%.3f\n",
               st->droplets_run, st->droplets_offmap,
               st->total_eroded, st->total_deposited, st->max_delta);
    }
}

// map a normalised value [0,1] to a glyph from the ramp.
static char ramp_glyph(float t) {
    static const char ramp[] = " .:-=+*#%@";
int n = (int)(sizeof(ramp) - 2);
int i = (int)(t * (float)n + 0.5f);
if (i < 0) i = 0;
if (i > n) i = n;
return ramp[i];
}

void erosion_debug_dump_relief(const erosion_field *f) {
    float lo = 1e30f, hi = -1e30f;
    for (int i = 0; i < EROSION_CELLS; i++) {
        if (f->height[i] < lo) lo = f->height[i];
        if (f->height[i] > hi) hi = f->height[i];
    }
    float span = (hi - lo) > 1e-4f ? (hi - lo) : 1.0f;

    printf("[erosion] relief (%d,%d):\n", f->ox, f->oz);
    for (int z = EROSION_PAD; z < EROSION_DIM_Z - EROSION_PAD; z++) {
        putchar(' '); putchar(' ');
        for (int x = EROSION_PAD; x < EROSION_DIM_X - EROSION_PAD; x++) {
            float t = (f->height[erosion_idx(x, z)] - lo) / span;
            putchar(ramp_glyph(t));
        }
        putchar('\n');
    }
}

void erosion_debug_dump_channels(const erosion_flux *fx, float thresh) {
    printf("[erosion] channels (thresh=%.2f):\n", thresh);
for (int z = EROSION_PAD;
z < EROSION_DIM_Z - EROSION_PAD;
for (int i = 0;
i < EROSION_CELLS;
return bad;
}
