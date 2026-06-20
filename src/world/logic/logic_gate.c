#include "logic_gate.h"
#include "logic_block.h"
#include "logic_signal.h"
#include "logic_dir.h"
#include <stddef.h>

uint8_t logic_gate_eval(logic_gate_kind kind, uint8_t a, uint8_t b) {
    int x = logic_is_on(a);
    int y = logic_is_on(b);
    int out;
    switch (kind) {
        case LOGIC_GATE_AND: out =  (x && y); break;
        case LOGIC_GATE_OR:  out =  (x || y); break;
        case LOGIC_GATE_XOR: out =  (x ^  y); break;
        case LOGIC_GATE_NOT: out =  (!x);     break; // ignores b by convention
        default:             out = 0;         break;
    }
    return out ? LOGIC_MAX : 0;
}

// read the level a neighbouring cell would feed into this face. a wire feeds
// its (already attenuated) output; a source/gate/repeater feeds its output if
// it points back at us, otherwise nothing. we keep it simple: any powered
// neighbour that isn't the gate's own output face counts.
static uint8_t feed_from(logic_grid *g, int x, int y, int z) {
    logic_cell *n = logic_grid_get(g, x, y, z);
    if (!n) return 0;
    return n->power;
}

void logic_gate_read_inputs(logic_grid *g, const logic_cell *c,
                            uint8_t *a, uint8_t *b) {
    *a = 0;
    *b = 0;

    logic_dir face = (logic_dir)c->facing;
    int gk = logic_block_gate_kind(c->kind);

    if (gk == LOGIC_GATE_NOT) {
        // inverter: input is the block directly behind the output face.
        logic_dir back = logic_dir_opposite(face);
        int bx, by, bz;
        logic_dir_step(back, c->x, c->y, c->z, &bx, &by, &bz);
        *a = feed_from(g, bx, by, bz);
        return;
    }

    // two-input gate: read the two faces perpendicular to the output.
    logic_dir left, right;
    logic_dir_perp(face, &left, &right);
    if (left != LOGIC_DIR_COUNT) {
        int lx, ly, lz;
        logic_dir_step(left, c->x, c->y, c->z, &lx, &ly, &lz);
        *a = feed_from(g, lx, ly, lz);
    }
    if (right != LOGIC_DIR_COUNT) {
        int rx, ry, rz;
        logic_dir_step(right, c->x, c->y, c->z, &rx, &ry, &rz);
        *b = feed_from(g, rx, ry, rz);
    }
}

uint8_t logic_gate_compute(logic_grid *g, const logic_cell *c) {
    int gk = logic_block_gate_kind(c->kind);
    if (gk == LOGIC_GATE_COUNT) return 0; // not a gate, defensive

    uint8_t a, b;
    logic_gate_read_inputs(g, c, &a, &b);
    return logic_gate_eval((logic_gate_kind)gk, a, b);
}

const char *logic_gate_name(logic_gate_kind kind) {
    switch (kind) {
        case LOGIC_GATE_AND: return "AND";
        case LOGIC_GATE_OR:  return "OR";
        case LOGIC_GATE_XOR: return "XOR";
        case LOGIC_GATE_NOT: return "NOT";
        default:             return "?";
    }
}
