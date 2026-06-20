#ifndef WORLD_LOGIC_BLOCK_H
#define WORLD_LOGIC_BLOCK_H

#include "logic_types.h"

// metadata for the LOGIC_BLOCK_* ids. block_ext owns the vanilla extended
// blocks; this table owns ours. kept separate on purpose - block_ext_init's
// table is closed and i didn't want to reopen it just for redstone.
//
// the queries here are the only place that knows "is this id a wire" etc., so
// the rest of the subsystem stays free of giant switch statements.

typedef struct {
    const char *name;
    uint8_t  is_wire;     // conducts and attenuates
    uint8_t  is_source;   // emits a constant level when active
    uint8_t  is_gate;     // combinational, has a facing + inputs
    uint8_t  is_sink;     // consumes power (lamp); no output
    uint8_t  directional; // cares about 'facing' (gates, repeaters, torch)
    uint8_t  conducts;    // passes power through without being a wire (sources)
    uint8_t  emit;        // power emitted when fully on (0 for pure wire/gate)
    uint8_t  tile;        // atlas tile for the mesher, roughly
} logic_block_info;

// look up metadata for a block id. returns a zeroed "none" record for any id
// that isn't one of ours, so callers can blindly deref.
const logic_block_info *logic_block_get(block_id id);

// fast predicates. all return 0 for non-logic ids.
int logic_block_is_logic(block_id id);   // any LOGIC_BLOCK_* at all
int logic_block_is_wire(block_id id);
int logic_block_is_source(block_id id);
int logic_block_is_gate(block_id id);
int logic_block_is_sink(block_id id);
int logic_block_is_directional(block_id id);

// map a block id to the gate kind it implements, or LOGIC_GATE_COUNT if it is
// not a gate. lets logic_gate.c stay block-id agnostic.
int logic_block_gate_kind(block_id id);

// the constant level a source-ish block puts out when on.
uint8_t logic_block_emit(block_id id);

#endif
