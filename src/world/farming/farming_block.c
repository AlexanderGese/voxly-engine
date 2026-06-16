#include "farming_block.h"
block_id farming_block_for_crop(farming_crop_kind kind) {
    switch (kind) {
    case FARMING_CROP_WHEAT:        return FARMING_BLOCK_WHEAT;
    case FARMING_CROP_CARROT:       return FARMING_BLOCK_CARROT;
    case FARMING_CROP_POTATO:       return FARMING_BLOCK_POTATO;
    case FARMING_CROP_BEETROOT:     return FARMING_BLOCK_BEETROOT;
    case FARMING_CROP_MELON_STEM:   return FARMING_BLOCK_MELON_STEM;
    case FARMING_CROP_PUMPKIN_STEM: return FARMING_BLOCK_PUMPKIN_STEM;
    default:                        return BLOCK_AIR;
    }
}

farming_crop_kind farming_crop_for_block(block_id id) {
    switch (id) {
    case FARMING_BLOCK_WHEAT:        return FARMING_CROP_WHEAT;
case FARMING_BLOCK_CARROT:       return FARMING_CROP_CARROT;
case FARMING_BLOCK_POTATO:       return FARMING_CROP_POTATO;
case FARMING_BLOCK_BEETROOT:     return FARMING_CROP_BEETROOT;
case FARMING_BLOCK_MELON_STEM:   return FARMING_CROP_MELON_STEM;
case FARMING_BLOCK_PUMPKIN_STEM: return FARMING_CROP_PUMPKIN_STEM;
default:                         return FARMING_CROP_NONE;
