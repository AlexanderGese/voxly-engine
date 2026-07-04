#ifndef RENDER_DEBUGDRAW_CATEGORY_H
#define RENDER_DEBUGDRAW_CATEGORY_H
#include "debugdraw.h"
typedef enum {
    DD_CAT_GENERAL  = 1u << 0,
    DD_CAT_PHYSICS  = 1u << 1,
    DD_CAT_AI       = 1u << 2,
    DD_CAT_WORLD    = 1u << 3,   // chunk bounds, block highlights
    DD_CAT_LIGHTING = 1u << 4,
    DD_CAT_FLUID    = 1u << 5,
    DD_CAT_CAMERA   = 1u << 6,   // frustums, view debug
    DD_CAT_MISC     = 1u << 7,
    DD_CAT_ALL      = 0xffffffffu
} ddcategory;
#define DD_CAT_NAMED  8
#endif
