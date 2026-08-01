#include "mspawn_density.h"
#include "mspawn_rules.h"
#include <math.h>
#include <string.h>

// floor-divide world coord to cell coord. C truncates toward zero which is
// wrong for negatives, so do it the long way.
static int cell_of(float w) {
    int b = (int)floorf(w);
    if (b >= 0) return b / MSPAWN_CELL;
    return -((-b + MSPAWN_CELL - 1) / MSPAWN_CELL);
}

// map a cell coord pair into the tracked window, or -1 if it falls outside.
static int grid_index(const mspawn_density *d, int cx, int cz) {
    int gx = cx - d->origin_cx;
    int gz = cz - d->origin_cz;
    if (gx < 0 || gx >= MSPAWN_GRID_DIM) return -1;
    if (gz < 0 || gz >= MSPAWN_GRID_DIM) return -1;
    return gz * MSPAWN_GRID_DIM + gx;
}

// category for an entity type via the rule table. types not in the table get
// lumped as ambient so stray entities still count against *something*.
static mspawn_category cat_of(entity_type t) {
    const mspawn_entry *e = mspawn_entry_for(t);
    return e ? e->category : MSPAWN_CAT_AMBIENT;
}

void mspawn_density_init(mspawn_density *d) {
    memset(d, 0, sizeof *d);
    d->cat_cap[MSPAWN_CAT_PASSIVE] = MSPAWN_CAP_PASSIVE;
    d->cat_cap[MSPAWN_CAT_HOSTILE] = MSPAWN_CAP_HOSTILE;
    d->cat_cap[MSPAWN_CAT_AMBIENT] = MSPAWN_CAP_AMBIENT;
}

void mspawn_density_rebuild(mspawn_density *d, const mob_registry *mr,
                            vec3 player_pos) {
    memset(d->cat_count, 0, sizeof d->cat_count);
    memset(d->cell, 0, sizeof d->cell);

    // center the window on the player so the grid follows them around.
    int pcx = cell_of(player_pos.x);
    int pcz = cell_of(player_pos.z);
    d->origin_cx = pcx - MSPAWN_GRID_DIM / 2;
    d->origin_cz = pcz - MSPAWN_GRID_DIM / 2;

    for (int i = 0; i < mr->count; i++) {
        const entity *e = &mr->list[i];
        if (!e->alive) continue;
        mspawn_category cat = cat_of(e->type);
        d->cat_count[cat]++;

        int idx = grid_index(d, cell_of(e->pos.x), cell_of(e->pos.z));
        if (idx >= 0 && d->cell[cat][idx] < 255)
            d->cell[cat][idx]++;
    }
}

int mspawn_density_has_room(const mspawn_density *d, mspawn_category cat) {
    if (cat < 0 || cat >= MSPAWN_CAT_COUNT) return 0;
    return d->cat_count[cat] < d->cat_cap[cat];
}

int mspawn_density_cell_ok(const mspawn_density *d, mspawn_category cat,
                           vec3 pos) {
    if (cat < 0 || cat >= MSPAWN_CAT_COUNT) return 0;
    int idx = grid_index(d, cell_of(pos.x), cell_of(pos.z));
    if (idx < 0) return 0;            // off-window: assume crowded, dont spawn
    return d->cell[cat][idx] < MSPAWN_CELL_MAX;
}

void mspawn_density_account(mspawn_density *d, mspawn_category cat, vec3 pos) {
    if (cat < 0 || cat >= MSPAWN_CAT_COUNT) return;
    d->cat_count[cat]++;
    int idx = grid_index(d, cell_of(pos.x), cell_of(pos.z));
    if (idx >= 0 && d->cell[cat][idx] < 255)
        d->cell[cat][idx]++;
}
