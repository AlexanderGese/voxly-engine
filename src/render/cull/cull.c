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
}

// chunk-space distance squared from camera to a chunk center, measured in
// chunk units so the lod bands line up regardless of CHUNK_SIZE.
static float chunk_dist_sq(vec3 cam, vec3 center) {
    float dx = (cam.x - center.x) / (float)CHUNK_SIZE_X;
    float dz = (cam.z - center.z) / (float)CHUNK_SIZE_Z;
    // y is mostly ignored for chunk lod — columns are full height — but a
    // little vertical weight keeps high-flying cameras from over-detailing.
    float dy = (cam.y - center.y) / (float)CHUNK_SIZE_Y;
    return dx*dx + dz*dz + 0.25f * dy*dy;
}

// gather phase: build cull_items for every loaded chunk, applying frustum
// + distance rejects up front so the sort/occlusion sees a smaller set.
static void gather(cull_ctx *cc, world *w, vec3 cam) {
    float max_d2 = cc->render_dist_chunks * cc->render_dist_chunks;
    // include a fudge ring so chunks half-in still draw at the edge.
    max_d2 = (cc->render_dist_chunks + 1.0f) * (cc->render_dist_chunks + 1.0f);

    for (world_node *n = w->head; n; n = n->next) {
        chunk *c = n->c;
        if (!c) continue;

        cull_item it;
        it.c       = c;
        it.bounds  = cull_chunk_bounds(c);
        it.center  = cull_aabb_center(it.bounds);
        it.dist_sq = chunk_dist_sq(cam, it.center);
        it.lod     = CULL_LOD0;
        it.reason  = CULL_REASON_VISIBLE;
        it.sort_key = 0;

        // empty chunks never draw, but we still log them for the overlay.
        if (c->vertex_count <= 0) {
            it.reason = CULL_REASON_EMPTY;
            cull_draw_list_add(&cc->list, it);
            continue;
        }

        // hard distance cutoff (chunk units).
        if (it.dist_sq > max_d2) {
            it.reason = CULL_REASON_DISTANCE;
            cull_draw_list_add(&cc->list, it);
            continue;
        }

        // frustum reject. use the cached cull_frustum.
        if (cc->do_frustum && !cull_frustum_visible(&cc->frustum, it.bounds)) {
            it.reason = CULL_REASON_FRUSTUM;
            cull_draw_list_add(&cc->list, it);
            continue;
        }

        // lod pick (distance based, stable-ish). reuse the chunk's stored
        // lod if we have one — here we just use fresh since chunk doesnt
        // carry it; prev_lod_valid stays 0 until a meshing pass tracks it.
        if (cc->do_lod) {
            it.lod = cull_lod_pick(&cc->lod, it.dist_sq);
        }

        it.reason = CULL_REASON_VISIBLE;
        cull_draw_list_add(&cc->list, it);
    }
}

// compact the item array in place so only VISIBLE items remain, keeping
// order. returns the new count. the dropped items already have their
// reason recorded for the tally, which we run before compacting.
static int compact_visible(cull_item *items, int count) {
    int w = 0;
    for (int i = 0; i < count; i++) {
        if (items[i].reason == CULL_REASON_VISIBLE) {
            if (w != i) items[w] = items[i];
            w++;
        }
    }
    return w;
}

int cull_run(cull_ctx *cc, world *w, mat4 vp, vec3 cam_pos) {
    cull_frustum_from_vp(&cc->frustum, vp, cam_pos);
    cull_draw_list_reset(&cc->list);

    gather(cc, w, cam_pos);

    cull_item *items = cc->list.items;
    int n = (int)darr_len(items);
    if (n == 0) {
        cull_draw_list_tally(&cc->list);
        return 0;
    }

    // sort EVERYTHING front-to-back first. the rejected entries sort too
    // but thats fine, they get compacted out below. occlusion needs the
    // visible ones in near->far order and a full sort guarantees that.
    cull_item *scratch = cull_draw_list_scratch(&cc->list);
    cull_sort_near_first(items, n, scratch);

    // occlusion walks near->far, but only over visible items. instead of a
    // separate array we step every item and skip non-visible ones.
    if (cc->do_occlusion) {
        cull_occlusion_begin(&cc->occ, vp, 1);
        for (int i = 0; i < n; i++) {
            if (items[i].reason == CULL_REASON_VISIBLE) {
                cull_occlusion_step(&cc->occ, &items[i]);
            }
        }
    }

    // tally before compaction so dropped reasons are still counted.
    cull_draw_list_tally(&cc->list);

    // compact down to the survivors and shrink the darray length.
    int survivors = compact_visible(items, n);
    if (cc->list.items) darr_hdr(cc->list.items)->len = (size_t)survivors;

    return survivors;
}

const cull_item *cull_visible(const cull_ctx *cc, int *count_out) {
    if (count_out) *count_out = (int)darr_len(cc->list.items);
    return cc->list.items;
}

const cull_stats *cull_last_stats(const cull_ctx *cc) {
    return &cc->list.stats;
}
