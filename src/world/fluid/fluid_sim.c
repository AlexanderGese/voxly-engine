#include "fluid_sim.h"
#include "water_spread.h"
#include "lava_spread.h"

#include <string.h>

void voxl_fluid_sim_init(voxl_fluid_sim *s) {
    if (!s) return;
    memset(s, 0, sizeof(*s));
    voxl_fluid_grid_clear(&s->grid);
    voxl_fluid_sources_clear(&s->sources);
    s->tick = 0;
    s->cold = false;
    s->last_changed = 0;
}

int voxl_fluid_sim_add_source(voxl_fluid_sim *s, int x, int y, int z, uint8_t kind) {
    if (!s) return -1;
    int slot = voxl_fluid_source_add(&s->sources, x, y, z, kind);
    // seed the cell right away so it shows up before the first step
    voxl_fluid_cell *c = voxl_fluid_at(&s->grid, x, y, z);
    if (c) {
        c->kind = kind;
        c->level = VOXL_FLUID_FULL;
        c->falling = 0;
    }
    return slot;
}

bool voxl_fluid_sim_remove_source(voxl_fluid_sim *s, int x, int y, int z) {
    if (!s) return false;
    return voxl_fluid_source_remove(&s->sources, x, y, z);
}

void voxl_fluid_sim_set_solid(voxl_fluid_sim *s, int x, int y, int z, bool solid) {
    if (!s) return;
    voxl_fluid_set_solid(&s->grid, x, y, z, solid);
}

int voxl_fluid_sim_step(voxl_fluid_sim *s) {
    if (!s) return 0;
    s->tick++;

    // 1. keep sources topped up
    voxl_fluid_sources_apply(&s->sources, &s->grid);

    // 2. move the fluids
    int changed = 0;
    changed += voxl_fluid_water_step(&s->grid);
    changed += voxl_fluid_lava_step_gated(&s->grid, s->tick);

    // 3. environmental reactions
    voxl_fluid_interact_result ir = voxl_fluid_resolve_contact(&s->grid);
    changed += ir.hardened;
    changed += voxl_fluid_freeze_pass(&s->grid, s->cold);
    changed += voxl_fluid_evaporate_pass(&s->grid);

    // 4. new still-water sources forming in pools
    changed += voxl_fluid_sources_form(&s->sources, &s->grid);

    s->last_changed = changed;
    return changed;
}

const voxl_fluid_cell *voxl_fluid_sim_cell(const voxl_fluid_sim *s,
                                           int x, int y, int z) {
    if (!s) return NULL;
    return voxl_fluid_at_const(&s->grid, x, y, z);
}
