#include "logic_propagate.h"
#include "logic_block.h"
#include "logic_signal.h"
#include "logic_dir.h"
#include "../../util/darray.h"
#include <stddef.h>

// a flood frontier entry: a wire cell and the level it's being lit to.
typedef struct {
    logic_cell *cell;
    uint8_t     level;
} prop_node;

// is this cell an emitter that *feeds* wires (i.e. has a non-zero output and
// isn't itself a wire)? wires are relaxed by the flood; everything else is a
// fixed source for it.
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

    for (int d = 0; d < LOGIC_DIR_COUNT; d++) {
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
    for (int d = 0; d < LOGIC_DIR_COUNT; d++) {
        int nx, ny, nz;
        logic_dir_step((logic_dir)d, wire->x, wire->y, wire->z, &nx, &ny, &nz);
        logic_cell *n = logic_grid_get(g, nx, ny, nz);
        if (!n) continue;
        best = logic_max2(best, logic_attenuate(n->power));
    }
    return best;
}

int logic_propagate_wires(logic_grid *g) {
    int changed = 0;

    // 1. clear all wires.
    logic_grid_each(g, reset_wires, &changed);

    // 2. collect the initial frontier from every emitter. we have to iterate
    // the grid to find emitters; gather their wire neighbours.
    prop_node *front = NULL;
    // logic_grid_each doesn't thread a grid pointer, so walk the map directly.
    hm_iter it;
    uint64_t key;
    void *val;
    hm_iter_init(&it, &g->map);
    while (hm_iter_next(&it, &key, &val)) {
        seed_from_emitter(g, (logic_cell *)val, &front);
    }

    // 3. relax the frontier. classic BFS-by-level: pop, and for each wire
    // neighbour try to light it one weaker. power strictly decreases so this
    // terminates without a visited set.
    size_t i = 0;
    while (i < darr_len(front)) {
        prop_node pn = front[i++];
        uint8_t spread = logic_attenuate(pn.level);
        if (spread == 0) continue;

        for (int d = 0; d < LOGIC_DIR_COUNT; d++) {
            int nx, ny, nz;
            logic_dir_step((logic_dir)d, pn.cell->x, pn.cell->y, pn.cell->z,
                           &nx, &ny, &nz);
            logic_cell *n = logic_grid_get(g, nx, ny, nz);
            if (!n || !logic_block_is_wire(n->kind)) continue;
            if (spread > n->power) {
                n->power = spread;
                prop_node next = { n, spread };
                darr_push(front, next);
            }
        }
    }

    darr_free(front);
    return changed;
}
