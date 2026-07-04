#ifndef RENDER_DEBUGDRAW_CATEGORY_H
#define RENDER_DEBUGDRAW_CATEGORY_H

#include "debugdraw.h"

// category filtering. every primitive is tagged with the "current" category
// (a single bit), and a runtime mask decides which categories actually draw.
// this is how you keep physics colliders, ai paths and chunk bounds on
// separate toggles without recompiling. the f3 menu flips bits in the mask.
//
// the category is push/pop scoped so helper functions can set their own
// without clobbering the caller's:
//
// debugdraw_push_category(dd, DD_CAT_PHYSICS);
// debugdraw_entity(dd, e, c);
// debugdraw_pop_category(dd);

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

// number of named categories above (used for the toggle ui loop)
#define DD_CAT_NAMED  8

// set the mask of categories that are allowed to draw. anything whose bit
// is clear gets dropped at emit time.
void debugdraw_set_mask(debugdraw *dd, uint32_t mask);
uint32_t debugdraw_mask(const debugdraw *dd);

// flip a single category on/off in the mask
void debugdraw_toggle_category(debugdraw *dd, ddcategory cat);
int  debugdraw_category_on(const debugdraw *dd, ddcategory cat);

// push/pop the active category. nestable up to DD_CAT_STACK_MAX.
void debugdraw_push_category(debugdraw *dd, ddcategory cat);
void debugdraw_pop_category(debugdraw *dd);

// is the current category visible under the mask? emit paths check this.
int  debugdraw_category_visible(const debugdraw *dd);

// human readable name for the toggle ui. returns "?" for unknown bits.
const char *debugdraw_category_name(ddcategory cat);

#endif
