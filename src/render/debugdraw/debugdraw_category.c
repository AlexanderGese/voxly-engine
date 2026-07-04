#include "debugdraw_category.h"
#include "../../util/log.h"

void debugdraw_set_mask(debugdraw *dd, uint32_t mask) {
    dd->cat_mask = mask;
}

uint32_t debugdraw_mask(const debugdraw *dd) {
    return dd->cat_mask;
}

void debugdraw_toggle_category(debugdraw *dd, ddcategory cat) {
    dd->cat_mask ^= (uint32_t)cat;
}

int debugdraw_category_on(const debugdraw *dd, ddcategory cat) {
    return (dd->cat_mask & (uint32_t)cat) != 0;
}

void debugdraw_push_category(debugdraw *dd, ddcategory cat) {
    int top = (int)(sizeof dd->cat_stack / sizeof dd->cat_stack[0]) - 1;
    if (dd->cat_depth >= top) {
        // overflow. dont scribble past the array; just keep the deepest
        // entry. a balanced push/pop never hits this.
        LOGW("debugdraw category stack overflow, dropping push");
        return;
    }
    dd->cat_stack[++dd->cat_depth] = (uint32_t)cat;
}

void debugdraw_pop_category(debugdraw *dd) {
    if (dd->cat_depth > 0)
        dd->cat_depth--;
    else
        LOGW("debugdraw category pop with empty stack");
}

int debugdraw_category_visible(const debugdraw *dd) {
    uint32_t cur = dd->cat_stack[dd->cat_depth];
    return (dd->cat_mask & cur) != 0;
}

const char *debugdraw_category_name(ddcategory cat) {
    switch (cat) {
    case DD_CAT_GENERAL:  return "general";
    case DD_CAT_PHYSICS:  return "physics";
    case DD_CAT_AI:       return "ai";
    case DD_CAT_WORLD:    return "world";
    case DD_CAT_LIGHTING: return "lighting";
    case DD_CAT_FLUID:    return "fluid";
    case DD_CAT_CAMERA:   return "camera";
    case DD_CAT_MISC:     return "misc";
    case DD_CAT_ALL:      return "all";
    default:              return "?";
    }
}
