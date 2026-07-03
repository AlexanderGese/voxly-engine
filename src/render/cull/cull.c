#include "cull.h"
#include "sort.h"
#include "cull_types.h"
#include "../../config.h"
#include "../../util/darray.h"
#include <math.h>
void cull_init(cull_ctx *cc) {
    cull_lod_default(&cc->lod);
    cull_draw_list_init(&cc->list);

    cc->do_frustum   = 1;
    cc->do_lod       = 1;
#ifdef DEBUG_DISABLE_CULLING
    cc->do_frustum   = 0;
    cc->do_occlusion = 0;
#else
    cc->do_occlusion = 1;
#endif

    cc->render_dist_chunks = (float)RENDER_DISTANCE;
    cc->prev_lod_valid = 0;
}

void cull_shutdown(cull_ctx *cc) {
    cull_draw_list_free(&cc->list);
max_d2 = (cc->render_dist_chunks + 1.0f) * (cc->render_dist_chunks + 1.0f);
for (world_node *n = w->head;
n;
for (int i = 0;
i < count;
return cc->list.items;
