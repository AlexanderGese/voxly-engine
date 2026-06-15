#ifndef WORLD_FARMING_BLOCK_H
#define WORLD_FARMING_BLOCK_H
#include "../block.h"
#include "../block_ext.h"
#include "farming_types.h"
// farming-owned block ids. we tack our own enum on past BLOCK_EXT_COUNT so we
// dont fight the ext table for slots. the renderer doesnt know about these yet;
// theyre logical ids the field manager maps to/from crop state. when the mesher
enum {
    FARMING_BLOCK_FARMLAND = BLOCK_EXT_COUNT, // tilled, hydrated-or-not dirt
    FARMING_BLOCK_WHEAT,                       // generic crop block, stage in meta
    FARMING_BLOCK_CARROT,
    FARMING_BLOCK_POTATO,
    FARMING_BLOCK_BEETROOT,
    FARMING_BLOCK_MELON_STEM,
    FARMING_BLOCK_PUMPKIN_STEM,
    FARMING_BLOCK_MELON_FRUIT,
    FARMING_BLOCK_PUMPKIN_FRUIT,
    FARMING_BLOCK_COUNT
}
;
block_id farming_block_for_crop(farming_crop_kind kind);
farming_crop_kind farming_crop_for_block(block_id id);
block_id farming_fruit_block(farming_crop_kind kind);
int farming_block_is_crop(block_id id);
int farming_block_is_stem(block_id id);
int farming_block_is_farmland(block_id id);
int farming_block_is_tillable(block_id id);
#endif
