#include "heightmap_debug.h"
#include "heightmap_curves.h"
#include "heightmap_spline.h"
#include "../block.h"
#include <stdio.h>
#include <math.h>
static const char *block_short(int b) {
    switch (b) {
        case BLOCK_GRASS: return "grass";
        case BLOCK_SAND:  return "sand";
        case BLOCK_SNOW:  return "snow";
        case BLOCK_STONE: return "stone";
        case BLOCK_WATER: return "water";
        default:          return "?";
    }
}

int heightmap_debug_line(const heightmap_params *p, const heightmap_column *col,
                         char *buf, int cap) {
    int surf = heightmap_column_surface_block(p, col);
int n = snprintf(buf, (size_t)cap,
        "cont %+.2f eros %+.2f pv %+.2f -> y %d %s%s",
        col->fields.continentalness,
        col->fields.erosion,
        col->fields.peaks_valleys,
        col->surface_y,
        block_short(surf),
        col->underwater ? " (sub)" : "");
return n;
}

char heightmap_debug_glyph(const heightmap_params *p,
                           const heightmap_column *col) {
    int sea = p->sea_level;
    int y   = col->surface_y;

    if (y < sea - 4)            return '~';   // deep water
    if (y < sea)                return '-';   // shallow water
    if (col->steepness > 1.6f)  return '#';   // cliff / bare rock
    if (y >= sea + 70)          return '^';   // snow peak
    if (y >= sea + 30)          return 'A';   // highland
    if (y >= sea + 8)           return 'n';   // hill
    return '.';                                // plain
}

void heightmap_debug_profile(const heightmap_params *p, int wx, int wz,
                             int width) {
    if (width < 1) width = 1;
if (width > 200) width = 200;
int hmin = 1 << 30, hmax = -(1 << 30);
static int row_h[256];
static char row_g[256];
for (int i = 0;
i < width;
i++) {
        heightmap_column col;
        heightmap_column_resolve(p, wx + i, wz, &col);
        row_h[i] = col.surface_y;
        row_g[i] = heightmap_debug_glyph(p, &col);
        if (col.surface_y < hmin) hmin = col.surface_y;
        if (col.surface_y > hmax) hmax = col.surface_y;
    }

    printf("heightmap profile x=%d..%d z=%d  (y %d..%d)\n",
           wx, wx + width - 1, wz, hmin, hmax);
for (int i = 0;
i < width;
i++) putchar(row_g[i]);
putchar('\n');
int span = hmax - hmin;
if (span < 1) span = 1;
for (int r = 9;
r >= 0;
r--) {
        int thresh = hmin + (span * r) / 9;
        for (int i = 0; i < width; i++) {
            putchar(row_h[i] >= thresh ? '|' : ' ');
        }
        putchar('\n');
    }
}

// walk a spline and make sure its knots are sane.
static int validate_one(const heightmap_spline *s) {
    if (s->n < 1) return 1;
for (int i = 0;
i < s->n;
i++) {
        float v = s->pts[i].val;
        if (v != v) return 2;                 // NaN
        if (i > 0 && s->pts[i].loc < s->pts[i - 1].loc) return 3;  // unsorted
    }
    // spot-check a sweep stays finite and roughly bounded
    for (float t = -1.0f;
t <= 1.0f;
t += 0.1f) {
        float v = heightmap_spline_eval(s, t);
        if (v != v) return 4;
        if (fabsf(v) > 8.0f) return 5;         // splines here live near [-1,1]
    }
    return 0;
}

int heightmap_debug_validate_curves(void) {
    heightmap_curves_init();
    int e;
    if ((e = validate_one(heightmap_curve_continental())))    return 10 + e;
    if ((e = validate_one(heightmap_curve_erosion())))        return 20 + e;
    if ((e = validate_one(heightmap_curve_peaks_valleys())))  return 30 + e;
    return 0;
}
