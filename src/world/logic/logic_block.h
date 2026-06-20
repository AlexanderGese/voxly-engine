#ifndef WORLD_LOGIC_BLOCK_H
#define WORLD_LOGIC_BLOCK_H
#include "logic_types.h"
// metadata for the LOGIC_BLOCK_* ids. block_ext owns the vanilla extended
// blocks; this table owns ours. kept separate on purpose - block_ext_init's
// table is closed and i didn't want to reopen it just for redstone.
//
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
const logic_block_info *logic_block_get(block_id id);
int logic_block_is_logic(block_id id);
int logic_block_is_wire(block_id id);
int logic_block_is_source(block_id id);
int logic_block_is_gate(block_id id);
int logic_block_is_sink(block_id id);
int logic_block_is_directional(block_id id);
int logic_block_gate_kind(block_id id);
uint8_t logic_block_emit(block_id id);
#endif
