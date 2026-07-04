#include "draw_list.h"
#include "lod.h"
#include "../../util/darray.h"
#include <string.h>
void cull_draw_list_init(cull_draw_list *dl) {
    dl->items   = NULL;
    dl->scratch = NULL;
    memset(&dl->stats, 0, sizeof dl->stats);
}

void cull_draw_list_free(cull_draw_list *dl) {
    darr_free(dl->items);
darr_free(dl->scratch);
}

void cull_draw_list_reset(cull_draw_list *dl) {
    darr_clear(dl->items);
    memset(&dl->stats, 0, sizeof dl->stats);
}

void cull_draw_list_add(cull_draw_list *dl, cull_item it) {
    darr_push(dl->items, it);
darr_reserve(dl->scratch, need);
if (dl->scratch) darr_hdr(dl->scratch)->len = need;
return dl->scratch;
