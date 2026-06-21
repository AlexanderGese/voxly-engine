#ifndef WORLD_LOGIC_TYPES_H
#define WORLD_LOGIC_TYPES_H
#include "../world.h"
#include "../chunk.h"
#include "../block.h"
#include "../block_ext.h"
#include "../../config.h"
#include <stdint.h>
// wire logic. a redstone-ish signal layer that lives on top of the block grid.
//
// the idea: certain blocks carry "power" 0..LOGIC_MAX. dust/wire spreads it and
// loses 1 per step. gates combine it. repeaters delay + clean it. everything
// runs off a scheduled update queue so a long line of repeaters ticks in order
// instead of all at once. it is NOT physically accurate redstone and i am not
// going to pretend it is, but the rules are self consistent.
// power range. mirrors MAX_LIGHT on purpose: same nibble tricks apply and the
// debug overlay can reuse the light ramp colours.
#define LOGIC_MAX        15
#define LOGIC_MIN         0
// wire only carries this far before it dies. keep <= LOGIC_MAX.
#define LOGIC_WIRE_RANGE 15
// a repeater's selectable delay, in ticks. four notches like the real thing.
#define LOGIC_REP_MIN_DELAY 1
#define LOGIC_REP_MAX_DELAY 4
// upper bound on cells re-evaluated in a single tick before we bail. a runaway
// clock shouldn't get to wedge the frame; it just spills into the next tick.
#define LOGIC_TICK_BUDGET 8192
// the wire-logic block ids. these continue the block_ext enum so they slot into
// the same world grid as everything else. we do NOT register them with
// block_ext_init (that table is closed); logic_block.c owns their metadata.
enum {
    LOGIC_BLOCK_BASE = BLOCK_EXT_COUNT,
    LOGIC_BLOCK_WIRE = LOGIC_BLOCK_BASE, // dust. spreads power, decays by 1/step
    LOGIC_BLOCK_SOURCE,                  // constant LOGIC_MAX emitter (a battery)
    LOGIC_BLOCK_LAMP,                    // sink. lit when powered (drives light)
    LOGIC_BLOCK_REPEATER,                // delay + signal restore, directional
    LOGIC_BLOCK_TORCH,                   // inverter mounted on a block
    LOGIC_BLOCK_GATE_AND,
    LOGIC_BLOCK_GATE_OR,
    LOGIC_BLOCK_GATE_XOR,
    LOGIC_BLOCK_GATE_NOT,
    LOGIC_BLOCK_BUTTON,                  // momentary source, self-resets
    LOGIC_BLOCK_LEVER,                   // latching source, toggled by hand
    LOGIC_BLOCK_COUNT
}
;
// combinational gate flavours. the truth tables live in logic_gate.c; this
// enum is shared vocabulary so logic_block.c can map a block id to a kind.
typedef enum {
    LOGIC_GATE_AND = 0,
    LOGIC_GATE_OR,
    LOGIC_GATE_XOR,
    LOGIC_GATE_NOT,
    LOGIC_GATE_COUNT
} logic_gate_kind;
// six axis directions. matches the face order used elsewhere in world/.
typedef enum {
    LOGIC_DIR_PX = 0, // +x
    LOGIC_DIR_NX,     // -x
    LOGIC_DIR_PY,     // +y
    LOGIC_DIR_NY,     // -y
    LOGIC_DIR_PZ,     // +z
    LOGIC_DIR_NZ,     // -z
    LOGIC_DIR_COUNT
} logic_dir;
// a single addressable logic cell. world-space coords so we can cross chunk
// borders without re-resolving the chunk pointer every neighbour lookup.
typedef struct {
    int32_t  x, y, z;
    block_id kind;     // which LOGIC_BLOCK_* this is
    uint8_t  power;    // current output power 0..LOGIC_MAX
    uint8_t  next;     // staged power for the next tick (double-buffer)
    uint8_t  facing;   // logic_dir the cell points (gates/repeaters/torch)
    uint8_t  delay;    // repeater delay setting, in ticks
    uint8_t  phase;    // repeater countdown / button hold timer
    uint8_t  flags;    // see LOGIC_CF_*
} logic_cell;
// cell flags.
enum {
    LOGIC_CF_LATCHED = 1 << 0, // lever/latch currently on
    LOGIC_CF_STAGED  = 1 << 1, // 'next' differs from 'power', needs a commit
    LOGIC_CF_VISITED = 1 << 2  // scratch bit for net walks
}
;
// packed key for the sparse grid. world coords fit comfortably; y is small.
static inline uint64_t logic_key(int x, int y, int z) {
    // x,z get 24 bits each, y gets 16. plenty for any sane world.
    uint64_t ux = (uint64_t)(uint32_t)(x & 0xFFFFFF);
    uint64_t uy = (uint64_t)(uint32_t)(y & 0xFFFF);
    uint64_t uz = (uint64_t)(uint32_t)(z & 0xFFFFFF);
    return ux | (uz << 24) | (uy << 48);
}
#endif
