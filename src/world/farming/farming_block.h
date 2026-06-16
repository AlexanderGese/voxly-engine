#ifndef WORLD_FARMING_BLOCK_H
#define WORLD_FARMING_BLOCK_H
#include "../block.h"
#include "../block_ext.h"
#include "farming_types.h"
// farming-owned block ids. we tack our own enum on past BLOCK_EXT_COUNT so we
// dont fight the ext table for slots. the renderer doesnt know about these yet;
// theyre logical ids the field manager maps to/from crop state. when the mesher
// eventually learns crops, point its plant table at these.
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
// the crop block id a species plants as. NONE -> BLOCK_AIR, which is how we
// signal "nothing here" to callers that only speak block ids.
block_id farming_block_for_crop(farming_crop_kind kind);
// inverse: which species does this crop block belong to. returns
// FARMING_CROP_NONE for anything thats not one of ours.
farming_crop_kind farming_crop_for_block(block_id id);
// the fruit block a stem spawns once it fruits. 0 for non-stem crops.
block_id farming_fruit_block(farming_crop_kind kind);
// classification helpers, the usual is_* family.
int farming_block_is_crop(block_id id);
int farming_block_is_stem(block_id id);
int farming_block_is_farmland(block_id id);
// a crop can only sit on farmland; this is what we check the block below for.
// dirt and grass both till into farmland, so they read as "tillable" too.
int farming_block_is_tillable(block_id id);
#endif
