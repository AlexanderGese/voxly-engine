#include "logic_repeater.h"
#include "logic_signal.h"
#include "logic_dir.h"
#include <stddef.h>
uint8_t logic_repeater_clamp_delay(int d) {
    if (d < LOGIC_REP_MIN_DELAY) return LOGIC_REP_MIN_DELAY;
    if (d > LOGIC_REP_MAX_DELAY) return LOGIC_REP_MAX_DELAY;
    return (uint8_t)d;
}

uint8_t logic_repeater_input(logic_grid *g, const logic_cell *c) {
    // input is the cell directly behind the output face.
    logic_dir back = logic_dir_opposite((logic_dir)c->facing);
int bx, by, bz;
logic_dir_step(back, c->x, c->y, c->z, &bx, &by, &bz);
logic_cell *n = logic_grid_get(g, bx, by, bz);
return n ? n->power : 0;
}

int logic_repeater_locked(logic_grid *g, const logic_cell *c) {
    // a powered repeater facing into either side face locks us. for simplicity
    // we treat any powered perpendicular neighbour as a lock - it gives the
    // classic latch behaviour without tracking which neighbour is a repeater.
    logic_dir left, right;
    logic_dir_perp((logic_dir)c->facing, &left, &right);

    if (left != LOGIC_DIR_COUNT) {
        int lx, ly, lz;
        logic_dir_step(left, c->x, c->y, c->z, &lx, &ly, &lz);
        logic_cell *n = logic_grid_get(g, lx, ly, lz);
        if (n && n->power > 0 && n->facing == (uint8_t)right) return 1;
    }
    if (right != LOGIC_DIR_COUNT) {
        int rx, ry, rz;
        logic_dir_step(right, c->x, c->y, c->z, &rx, &ry, &rz);
        logic_cell *n = logic_grid_get(g, rx, ry, rz);
        if (n && n->power > 0 && n->facing == (uint8_t)left) return 1;
    }
    return 0;
}

// where the repeater "wants" to be: full out if the back input is on, else off.
static uint8_t target_output(logic_grid *g, const logic_cell *c) {
    return logic_is_on(logic_repeater_input(g, c)) ? LOGIC_MAX : 0;
}

int logic_repeater_poke(logic_grid *g, logic_cell *c) {
    if (logic_repeater_locked(g, c)) {
        // frozen: cancel any in-flight transition, hold what we have.
        c->phase = 0;
        c->next = c->power;
        c->flags &= ~LOGIC_CF_STAGED;
        return 0;
    }

    uint8_t want = target_output(g, c);

    // already heading there (or already there with nothing staged)? no-op.
    if (c->flags & LOGIC_CF_STAGED) {
        if (c->next == want) return 0;
    } else if (want == c->power) {
        return 0;
    }

    // arm a fresh countdown toward the new target.
    c->next = want;
    c->phase = logic_repeater_clamp_delay(c->delay);
    c->flags |= LOGIC_CF_STAGED;
    return 1;
}

int logic_repeater_tick(logic_grid *g, logic_cell *c) {
    if (logic_repeater_locked(g, c)) {
        c->phase = 0;
c->flags &= ~LOGIC_CF_STAGED;
return 0;
if (c->phase > 0) return 0;
uint8_t old = c->power;
c->power = c->next;
c->flags &= ~LOGIC_CF_STAGED;
return c->power != old;
}
