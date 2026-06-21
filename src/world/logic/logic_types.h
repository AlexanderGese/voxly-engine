#ifndef WORLD_LOGIC_TYPES_H
#define WORLD_LOGIC_TYPES_H
#include "../world.h"
#include "../chunk.h"
#include "../block.h"
#include "../block_ext.h"
#include "../../config.h"
#include <stdint.h>
#define LOGIC_MAX        15
#define LOGIC_MIN         0
#define LOGIC_WIRE_RANGE 15
#define LOGIC_REP_MIN_DELAY 1
#define LOGIC_REP_MAX_DELAY 4
#define LOGIC_TICK_BUDGET 8192
typedef enum {
    LOGIC_GATE_AND = 0,
    LOGIC_GATE_OR,
    LOGIC_GATE_XOR,
    LOGIC_GATE_NOT,
    LOGIC_GATE_COUNT
} logic_gate_kind;
typedef enum {
    LOGIC_DIR_PX = 0, // +x
    LOGIC_DIR_NX,     // -x
    LOGIC_DIR_PY,     // +y
    LOGIC_DIR_NY,     // -y
    LOGIC_DIR_PZ,     // +z
    LOGIC_DIR_NZ,     // -z
    LOGIC_DIR_COUNT
} logic_dir;
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
#endif
