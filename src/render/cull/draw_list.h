#ifndef RENDER_CULL_DRAW_LIST_H
#define RENDER_CULL_DRAW_LIST_H
#include "cull_types.h"
// the visible-set container. just a growable array of cull_item plus a
// scratch buffer for the radix sort so we dont malloc every frame.
// backed by the engine's darray (stb-style header-in-front arrays).
typedef struct {
    cull_item *items;     // darray
    cull_item *scratch;   // darray, reused as radix scratch
    cull_stats stats;
} cull_draw_list;
void cull_draw_list_init(cull_draw_list *dl);
void cull_draw_list_free(cull_draw_list *dl);
// clear contents but keep the allocations. call at the top of each pass.
void cull_draw_list_reset(cull_draw_list *dl);
// append one item. caller has already filled bounds/center/dist_sq/lod.
void cull_draw_list_add(cull_draw_list *dl, cull_item it);
int  cull_draw_list_count(const cull_draw_list *dl);
// make sure scratch is at least as big as items, so sort can borrow it.
cull_item *cull_draw_list_scratch(cull_draw_list *dl);
// recompute the stats block from the current items + their reasons.
void cull_draw_list_tally(cull_draw_list *dl);
#endif
