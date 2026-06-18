#include "heightmap.h"
#include "heightmap_curves.h"

void heightmap_gen_init(heightmap_gen *g, uint32_t seed, int sea_level) {
    g->params = heightmap_default_params(seed, sea_level);
    // force the spline tables to build now, on the main thread, before any
    // worker asks for a column. the lazy init in curves.c isnt locked.
    heightmap_curves_init();
}

void heightmap_gen_init_params(heightmap_gen *g, const heightmap_params *p) {
    g->params = *p;
    heightmap_curves_init();
}

int heightmap_gen_height(const heightmap_gen *g, int wx, int wz) {
    return heightmap_column_height(&g->params, wx, wz);
}

void heightmap_gen_column(const heightmap_gen *g, int wx, int wz,
                          heightmap_column *out) {
    heightmap_column_resolve(&g->params, wx, wz, out);
}

void heightmap_gen_field(const heightmap_gen *g, int cx_world, int cz_world,
                         heightmap_field *out) {
    heightmap_field_build(out, &g->params, cx_world, cz_world);
}

int heightmap_gen_fill_column(const heightmap_gen *g, int wx, int wz,
                              int y0, int y1, uint8_t *out) {
    heightmap_column col;
    heightmap_column_resolve(&g->params, wx, wz, &col);
    return heightmap_fill_range(&g->params, &col, y0, y1, out);
}
