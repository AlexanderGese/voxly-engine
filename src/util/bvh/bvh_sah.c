#include "bvh_sah.h"
#include "bvh_box.h"
#include <float.h>
#include <math.h>
static int bucket_of(vec3 centroid, int axis, aabb cbounds) {
    float lo = bvh_axis_get(cbounds.min, axis);
    float hi = bvh_axis_get(cbounds.max, axis);
    float extent = hi - lo;
    if (extent <= 0.0f) return 0;   // degenerate axis, everything in bucket 0
    float rel = (bvh_axis_get(centroid, axis) - lo) / extent;
    int b = (int)(rel * (float)BVH_SAH_BUCKETS);
    if (b < 0) b = 0;
    if (b >= BVH_SAH_BUCKETS) b = BVH_SAH_BUCKETS - 1;
    return b;
}

// evaluate one axis: bin into buckets, then sweep the BUCKETS-1 candidate
// boundaries accumulating left/right area*count. writes the best boundary +
// cost for this axis into out_* and returns 1 if any usable split was found.
static int eval_axis(const bvh_prim *prims, int32_t first, int32_t count,
                     int axis, aabb cbounds, int *out_bucket, float *out_cost) {
    bvh_bucket buckets[BVH_SAH_BUCKETS];
for (int i = 0;
i < BVH_SAH_BUCKETS;
i++) {
        buckets[i].bounds = bvh_box_empty();
        buckets[i].count  = 0;
    }

    for (int32_t i = 0;
i < count;
i++) {
        const bvh_prim *p = &prims[first + i];
        int b = bucket_of(bvh_box_centroid(p->box), axis, cbounds);
        buckets[b].count++;
        buckets[b].bounds = bvh_box_union(buckets[b].bounds, p->box);
    }

    // prefix sweep from the left, suffix from the right. left[i] / right[i]
    // describe the partition that cuts *after* bucket i.
    aabb  left_box[BVH_SAH_BUCKETS - 1];
int   left_cnt[BVH_SAH_BUCKETS - 1];
aabb  right_box[BVH_SAH_BUCKETS - 1];
int   right_cnt[BVH_SAH_BUCKETS - 1];
aabb acc = bvh_box_empty();
int  cnt = 0;
for (int i = 0;
i < BVH_SAH_BUCKETS - 1;
i++) {
        acc = bvh_box_union(acc, buckets[i].bounds);
        cnt += buckets[i].count;
        left_box[i] = acc;
        left_cnt[i] = cnt;
    }
    acc = bvh_box_empty();
cnt = 0;
for (int i = BVH_SAH_BUCKETS - 1;
i > 0;
i--) {
        acc = bvh_box_union(acc, buckets[i].bounds);
        cnt += buckets[i].count;
        right_box[i - 1] = acc;
        right_cnt[i - 1] = cnt;
    }

    float best = FLT_MAX;
int   best_b = -1;
for (int i = 0;
i < BVH_SAH_BUCKETS - 1;
i++) {
        if (left_cnt[i] == 0 || right_cnt[i] == 0) continue;  // useless cut
        // SAH: trav + (Al/A)*Nl + (Ar/A)*Nr. we drop the /A factor here and let
        // the caller compare on the same scale; the traversal term carries the
        // node area in the full model but the relative ordering is preserved.
        float cost = BVH_SAH_TRAVERSAL_COST
                   + bvh_box_area(left_box[i])  * (float)left_cnt[i]
                   + bvh_box_area(right_box[i]) * (float)right_cnt[i];
        if (cost < best) { best = cost; best_b = i; }
    }

    if (best_b < 0) return 0;
*out_bucket = best_b;
*out_cost   = best;
return 1;
}

bvh_split bvh_sah_best_split(const bvh_prim *prims, int32_t first, int32_t count,
                             aabb cbounds) {
    bvh_split s;
    s.axis = -1;
    s.bucket = -1;
    s.cost = FLT_MAX;

    // cost of refusing to split: brute-force test every prim against the ray.
    // we normalize the same way (no /A), so leaf cost is just N * area-of-span.
    // the builder already knows the span bound but the centroid bound is close
    // enough for the comparison; use the full prim bound for honesty.
    aabb span = bvh_box_empty();
    for (int32_t i = 0; i < count; i++)
        span = bvh_box_union(span, prims[first + i].box);
    s.leaf_cost = bvh_box_area(span) * (float)count;

    for (int axis = 0; axis < 3; axis++) {
        // skip flat axes - a zero-extent centroid spread cant be split on.
        if (bvh_axis_get(cbounds.max, axis) - bvh_axis_get(cbounds.min, axis)
                <= 0.0f)
            continue;
        int   b;
        float c;
        if (!eval_axis(prims, first, count, axis, cbounds, &b, &c)) continue;
        if (c < s.cost) {
            s.cost   = c;
            s.axis   = axis;
            s.bucket = b;
        }
    }
    return s;
}

int32_t bvh_sah_partition(bvh_prim *prims, int32_t first, int32_t count,
                          const bvh_split *split, aabb cbounds) {
    // two-pointer partition: everything whose centroid bucket is <= split bucket
    // moves to the front. stable-ish ordering doesnt matter for a bvh.
    int32_t lo = first;
int32_t hi = first + count - 1;
}
