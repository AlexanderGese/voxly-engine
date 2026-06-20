#include "logic_block.h"
#include <stddef.h>

// one row per LOGIC_BLOCK_* id, indexed by (id - LOGIC_BLOCK_BASE). order MUST
// track the enum in logic_types.h. if you add a block there, add a row here or
// the lookup falls back to NONE and the block does nothing (which is at least a
// safe failure).
static const logic_block_info k_table[] = {
    // wire: dust. spreads, decays, no innate emission.
    { "wire",     1, 0, 0, 0, 0, 0,  0, 0 },
    // source: a battery. constant full output.
    { "source",   0, 1, 0, 0, 0, 1, LOGIC_MAX, 1 },
    // lamp: pure sink, drives light when fed.
    { "lamp",     0, 0, 0, 1, 0, 0,  0, 2 },
    // repeater: directional, restores to full, delays. treated as a gate-ish
    // directional but handled by its own state machine.
    { "repeater", 0, 0, 0, 0, 1, 0, LOGIC_MAX, 3 },
    // torch: inverter source, mounted, directional support.
    { "torch",    0, 1, 0, 0, 1, 1, LOGIC_MAX, 4 },
    // the four gates. directional, restore to full when their table says on.
    { "and",      0, 0, 1, 0, 1, 0, LOGIC_MAX, 5 },
    { "or",       0, 0, 1, 0, 1, 0, LOGIC_MAX, 6 },
    { "xor",      0, 0, 1, 0, 1, 0, LOGIC_MAX, 7 },
    { "not",      0, 0, 1, 0, 1, 0, LOGIC_MAX, 8 },
    // button: momentary source, self-resets after a hold.
    { "button",   0, 1, 0, 0, 0, 1, LOGIC_MAX, 9 },
    // lever: latching source, toggled by hand.
    { "lever",    0, 1, 0, 0, 0, 1, LOGIC_MAX, 10 },
};

static const logic_block_info k_none = {
    "none", 0, 0, 0, 0, 0, 0, 0, 0
};

const logic_block_info *logic_block_get(block_id id) {
    if (id < LOGIC_BLOCK_BASE || id >= LOGIC_BLOCK_COUNT) return &k_none;
    int row = (int)id - LOGIC_BLOCK_BASE;
    // bounds belt-and-suspenders: the enum and table could drift.
    if (row < 0 || row >= (int)(sizeof k_table / sizeof k_table[0]))
        return &k_none;
    return &k_table[row];
}

int logic_block_is_logic(block_id id) {
    return id >= LOGIC_BLOCK_BASE && id < LOGIC_BLOCK_COUNT;
}

int logic_block_is_wire(block_id id)        { return logic_block_get(id)->is_wire; }
int logic_block_is_source(block_id id)      { return logic_block_get(id)->is_source; }
int logic_block_is_gate(block_id id)        { return logic_block_get(id)->is_gate; }
int logic_block_is_sink(block_id id)        { return logic_block_get(id)->is_sink; }
int logic_block_is_directional(block_id id) { return logic_block_get(id)->directional; }

int logic_block_gate_kind(block_id id) {
    switch (id) {
        case LOGIC_BLOCK_GATE_AND: return LOGIC_GATE_AND;
        case LOGIC_BLOCK_GATE_OR:  return LOGIC_GATE_OR;
        case LOGIC_BLOCK_GATE_XOR: return LOGIC_GATE_XOR;
        case LOGIC_BLOCK_GATE_NOT: return LOGIC_GATE_NOT;
        default:                   return LOGIC_GATE_COUNT;
    }
}

uint8_t logic_block_emit(block_id id) {
    return logic_block_get(id)->emit;
}
