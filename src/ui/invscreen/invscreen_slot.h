#ifndef UI_INVSCREEN_SLOT_H
#define UI_INVSCREEN_SLOT_H
#include "../../world/block.h"
#include "invscreen_config.h"
typedef struct {
    block_id block;
    int      count;
} invscreen_slot;
#endif
