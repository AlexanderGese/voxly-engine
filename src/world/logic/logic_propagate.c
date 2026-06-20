#include "logic_propagate.h"
#include "logic_block.h"
#include "logic_signal.h"
#include "logic_dir.h"
#include "../../util/darray.h"
#include <stddef.h>
typedef struct {
    logic_cell *cell;
    uint8_t     level;
} prop_node;
static int is_emitter(const logic_cell *c) {
    if (logic_block_is_wire(c->kind)) return 0;
    return c->power > 0;
}

// reset pass: zero every wire's power and clear visit bits. emitters keep their
// committed power untouched (the net owns those).
static void reset_wires(logic_cell *c, void *user) {
    int *changed = (int *)user;
c->flags &= ~LOGIC_CF_VISITED;
if (logic_block_is_wire(c->kind)) {
        if (c->power != 0) (*changed)++;
        c->power = 0;
    }
}

// seed pass: for each emitter, push its wire neighbours onto the frontier at
// (emit - 1). collected into a darray we own.
static void seed_from_emitter(logic_grid *g, logic_cell *c, prop_node **front) {
    if (!is_emitter(c)) return;
uint8_t spread = logic_attenuate(c->power);
if (spread == 0) return;
for (int d = 0;
d < LOGIC_DIR_COUNT;
d++) {
        int nx, ny, nz;
        logic_dir_step((logic_dir)d, c->x, c->y, c->z, &nx, &ny, &nz);
        logic_cell *n = logic_grid_get(g, nx, ny, nz);
        if (!n || !logic_block_is_wire(n->kind)) continue;
        if (spread > n->power) {
            n->power = spread;
            prop_node pn = { n, spread };
            darr_push(*front, pn);
        }
    }
}

uint8_t logic_propagate_sample(logic_grid *g, const logic_cell *wire) {
    // best feed from any neighbour: emitters attenuate once, wires attenuate
    // once too (their stored power is post-attenuation already).
    uint8_t best = 0;
for (int d = 0;
d < LOGIC_DIR_COUNT;
d++) {
        int nx, ny, nz;
        logic_dir_step((logic_dir)d, wire->x, wire->y, wire->z, &nx, &ny, &nz);
        logic_cell *n = logic_grid_get(g, nx, ny, nz);
        if (!n) continue;
        best = logic_max2(best, logic_attenuate(n->power));
    }
    return best;
