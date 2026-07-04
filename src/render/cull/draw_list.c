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
}

int cull_draw_list_count(const cull_draw_list *dl) {
    return (int)darr_len(dl->items);
}

cull_item *cull_draw_list_scratch(cull_draw_list *dl) {
    size_t need = darr_len(dl->items);
    // grow scratch to match. we never read old scratch contents so we can
    // just bump the length to need without copying anything in.
    darr_reserve(dl->scratch, need);
    if (dl->scratch) darr_hdr(dl->scratch)->len = need;
    return dl->scratch;
}

void cull_draw_list_tally(cull_draw_list *dl) {
    cull_stats *s = &dl->stats;
    memset(s, 0, sizeof *s);

    size_t n = darr_len(dl->items);
    s->total = (int)n;

    for (size_t i = 0; i < n; i++) {
        cull_item *it = &dl->items[i];
        switch (it->reason) {
        case CULL_REASON_FRUSTUM:  s->frustum_culled++;   break;
        case CULL_REASON_OCCLUDED: s->occlusion_culled++; break;
        case CULL_REASON_DISTANCE: s->distance_culled++;  break;
        case CULL_REASON_EMPTY:    s->empty++;            break;
        case CULL_REASON_VISIBLE:
        default:
            s->drawn++;
            // rough triangle tally: verts/3 scaled by lod factor.
            if (it->c) {
                float scale = cull_lod_tri_scale(it->lod);
                s->tris_estimate += (int)((it->c->vertex_count / 3) * scale);
            }
            break;
        }
    }
}
