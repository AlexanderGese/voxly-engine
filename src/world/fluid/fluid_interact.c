#include "fluid_interact.h"
#include "lava_spread.h"
#include "fluid_level.h"

#include <string.h>

voxl_fluid_interact_result voxl_fluid_resolve_contact(voxl_fluid_grid *g) {
    voxl_fluid_interact_result r = { 0, 0, 0 };
    if (!g) return r;

    // snapshot so we test against pre-sweep state and dont chain-react in one
    // pass (a freshly hardened cell shouldnt instantly cool its neighbours).
    static voxl_fluid_grid voxl_fluid_contact_snap;
    memcpy(&voxl_fluid_contact_snap, g, sizeof(*g));

    for (int y = 0; y < VOXL_FLUID_GRID_N; y++) {
        for (int z = 0; z < VOXL_FLUID_GRID_N; z++) {
            for (int x = 0; x < VOXL_FLUID_GRID_N; x++) {
                const voxl_fluid_cell *c =
                    voxl_fluid_at_const(&voxl_fluid_contact_snap, x, y, z);
                if (!c || c->kind != VOXL_FLUID_LAVA || c->level == 0) continue;
                if (!voxl_fluid_lava_touches_water(&voxl_fluid_contact_snap, x, y, z))
                    continue;

                // lava becomes solid rock
                voxl_fluid_cell *live = voxl_fluid_at(g, x, y, z);
                if (live) {
                    live->kind = VOXL_FLUID_AIR;
                    live->level = 0;
                    live->falling = 0;
                }
                voxl_fluid_set_solid(g, x, y, z, true);
                r.hardened++;

                // eat the water that quenched it
                static const int ox[6] = { 1, -1, 0, 0, 0, 0 };
                static const int oy[6] = { 0, 0, 1, -1, 0, 0 };
                static const int oz[6] = { 0, 0, 0, 0, 1, -1 };
                for (int i = 0; i < 6; i++) {
                    voxl_fluid_cell *w =
                        voxl_fluid_at(g, x + ox[i], y + oy[i], z + oz[i]);
                    if (w && w->kind == VOXL_FLUID_WATER)
                        voxl_fluid_level_remove(w, VOXL_FLUID_FULL);
                }
            }
        }
    }
    return r;
}

int voxl_fluid_freeze_pass(voxl_fluid_grid *g, bool cold) {
    if (!g || !cold) return 0;
    int frozen = 0;
    for (int y = 0; y < VOXL_FLUID_GRID_N; y++) {
        for (int z = 0; z < VOXL_FLUID_GRID_N; z++) {
            for (int x = 0; x < VOXL_FLUID_GRID_N; x++) {
                voxl_fluid_cell *c = voxl_fluid_at(g, x, y, z);
                if (!c || c->kind != VOXL_FLUID_WATER) continue;
                if (c->level != VOXL_FLUID_FULL || c->falling) continue;
                // must be a surface cell (nothing fluid above)
                const voxl_fluid_cell *up = voxl_fluid_at_const(g, x, y + 1, z);
                if (up && !voxl_fluid_cell_empty(up)) continue;
                // turn into ice = solid for the sim's purposes
                c->kind = VOXL_FLUID_AIR;
                c->level = 0;
                voxl_fluid_set_solid(g, x, y, z, true);
                frozen++;
            }
        }
    }
    return frozen;
}

int voxl_fluid_evaporate_pass(voxl_fluid_grid *g) {
    if (!g) return 0;
    int gone = 0;
    static const int ox[4] = { 1, -1, 0, 0 };
    static const int oz[4] = { 0, 0, 1, -1 };
    for (int y = 0; y < VOXL_FLUID_GRID_N; y++) {
        for (int z = 0; z < VOXL_FLUID_GRID_N; z++) {
            for (int x = 0; x < VOXL_FLUID_GRID_N; x++) {
                voxl_fluid_cell *c = voxl_fluid_at(g, x, y, z);
                if (!c || c->kind != VOXL_FLUID_WATER) continue;
                if (c->level > 1 || c->falling) continue;
                // nothing above and no fluid neighbours -> a lonely puddle
                const voxl_fluid_cell *up = voxl_fluid_at_const(g, x, y + 1, z);
                if (up && !voxl_fluid_cell_empty(up)) continue;
                bool isolated = true;
                for (int i = 0; i < 4; i++) {
                    const voxl_fluid_cell *n =
                        voxl_fluid_at_const(g, x + ox[i], y, z + oz[i]);
                    if (n && !voxl_fluid_cell_empty(n)) { isolated = false; break; }
                }
                if (!isolated) continue;
                voxl_fluid_level_remove(c, VOXL_FLUID_FULL);
                gone++;
            }
        }
    }
    return gone;
}
