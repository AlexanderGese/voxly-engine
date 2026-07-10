#include "lt_grid.h"

#include "../../world/block.h"
#include <stdlib.h>
#include <string.h>

// ceil-div, because a 16-cell axis at step 8 is exactly 2 but at step 16 (if we
// ever went there) we'd still want 1 cell rather than 0. cheap insurance.
static int div_up(int a, int b) {
    return (a + b - 1) / b;
}

int lt_grid_init(lt_grid *g, int level) {
    if (level < 0) level = 0;
    if (level >= LT_LEVEL_COUNT) level = LT_LEVEL_COUNT - 1;

    int step = LT_STEP(level);

    g->level = level;
    g->step  = step;
    g->nx = div_up(LT_GRID_X, step);
    g->ny = div_up(LT_GRID_Y, step);
    g->nz = div_up(LT_GRID_Z, step);

    int vol = g->nx * g->ny * g->nz;
    g->cells = calloc((size_t)vol, sizeof(block_id));
    if (!g->cells) {
        // leave it in a defined empty state so free() is harmless
        g->nx = g->ny = g->nz = 0;
        return 0;
    }
    return 1;
}

void lt_grid_free(lt_grid *g) {
    if (!g) return;
    free(g->cells);
    g->cells = NULL;
    g->nx = g->ny = g->nz = 0;
}

int lt_grid_cell_solid(const lt_grid *g, int x, int y, int z) {
    block_id id = lt_grid_get(g, x, y, z);
    if (id == BLOCK_AIR) return 0;
    // opaque only — leaves and glass make holes in the far silhouette and look
    // worse than just dropping them. water is handled by the water pass anyway.
    return block_is_opaque(id);
}
