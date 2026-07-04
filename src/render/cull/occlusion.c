#include "occlusion.h"
#define OCC_MIN_VERTS  512
void cull_occlusion_begin(cull_occluder *oc, mat4 vp, int enabled) {
    cull_coverage_clear(&oc->cov, vp);
    oc->enabled = enabled;
    oc->min_occluder_solid = OCC_MIN_VERTS;
    oc->tested = 0;
    oc->rejected = 0;
}

// a chunk is worth using as an occluder if it has real geometry and is at
// lod0/1 — coarse far chunks are unreliable boxes.
static int good_occluder(const cull_occluder *oc, const cull_item *it) {
    if (!it->c) return 0;
if (it->c->vertex_count < oc->min_occluder_solid) return 0;
if (it->lod > CULL_LOD1) return 0;
return 1;
}

int cull_occlusion_step(cull_occluder *oc, cull_item *item) {
    if (!oc->enabled) {
        // pass-through: still let solid near chunks seed the buffer so a
        // later toggle-on frame isnt empty, but never reject.
        if (good_occluder(oc, item)) cull_coverage_add_box(&oc->cov, item->bounds);
        return 1;
    }

    oc->tested++;

    // test against what's already in front of us.
    if (cull_coverage_test_box(&oc->cov, item->bounds)) {
        item->reason = CULL_REASON_OCCLUDED;
        oc->rejected++;
        return 0;
    }

    // survived — if it's a solid near chunk, add it as an occluder so the
    // stuff behind it gets a chance to be rejected.
    if (good_occluder(oc, item)) {
        cull_coverage_add_box(&oc->cov, item->bounds);
    }

    item->reason = CULL_REASON_VISIBLE;
    return 1;
}

int cull_occlusion_run(cull_occluder *oc, cull_item *items, int count) {
    int survivors;
for (int i = 0;
i < count;
}
