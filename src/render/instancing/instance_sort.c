#include "instance_sort.h"

#include <stdlib.h>

// small-slice cutoff: below this we insertion sort (fewer branches, cache
// friendly), above it we recurse with quicksort. classic introsort-lite.
#define SORT_SMALL  24

vec3 instancing_sort_record_pos(const instancing_gpu_instance *rec) {
    // column-major 4x4: the translation is the 4th column, floats 12..14.
    return (vec3){ rec->model[12], rec->model[13], rec->model[14] };
}

// squared distance from a record to the camera. squared is fine — we only
// compare, never need the actual length.
static float dist_sq(const instancing_gpu_instance *rec, vec3 cam) {
    vec3 p = instancing_sort_record_pos(rec);
    float dx = p.x - cam.x, dy = p.y - cam.y, dz = p.z - cam.z;
    return dx * dx + dy * dy + dz * dz;
}

static void swap_rec(instancing_gpu_instance *a, instancing_gpu_instance *b) {
    instancing_gpu_instance t = *a;
    *a = *b;
    *b = t;
}

// `far_first` selects ordering: 1 -> descending distance (back to front),
// 0 -> ascending (front to back). keeping one comparator avoids duplicating
// the whole sort twice.
static int before(float da, float db, int far_first) {
    return far_first ? (da > db) : (da < db);
}

static void insertion(instancing_gpu_instance *recs, float *keys, int lo,
                      int hi, int far_first) {
    for (int i = lo + 1; i <= hi; ++i) {
        instancing_gpu_instance rv = recs[i];
        float rk = keys[i];
        int j = i - 1;
        while (j >= lo && before(rk, keys[j], far_first)) {
            recs[j + 1] = recs[j];
            keys[j + 1] = keys[j];
            j--;
        }
        recs[j + 1] = rv;
        keys[j + 1] = rk;
    }
}

static void quicksort(instancing_gpu_instance *recs, float *keys, int lo,
                      int hi, int far_first) {
    while (hi - lo > SORT_SMALL) {
        // median-of-three pivot to dodge the already-sorted worst case.
        int mid = lo + (hi - lo) / 2;
        if (before(keys[mid], keys[lo], far_first)) {
            swap_rec(&recs[lo], &recs[mid]);
            float t = keys[lo]; keys[lo] = keys[mid]; keys[mid] = t;
        }
        if (before(keys[hi], keys[lo], far_first)) {
            swap_rec(&recs[lo], &recs[hi]);
            float t = keys[lo]; keys[lo] = keys[hi]; keys[hi] = t;
        }
        if (before(keys[hi], keys[mid], far_first)) {
            swap_rec(&recs[mid], &recs[hi]);
            float t = keys[mid]; keys[mid] = keys[hi]; keys[hi] = t;
        }
        float pivot = keys[mid];

        int i = lo, j = hi;
        while (i <= j) {
            while (before(keys[i], pivot, far_first)) i++;
            while (before(pivot, keys[j], far_first)) j--;
            if (i <= j) {
                swap_rec(&recs[i], &recs[j]);
                float t = keys[i]; keys[i] = keys[j]; keys[j] = t;
                i++; j--;
            }
        }

        // recurse into the smaller side, loop on the larger — bounds stack.
        if (j - lo < hi - i) {
            quicksort(recs, keys, lo, j, far_first);
            lo = i;
        } else {
            quicksort(recs, keys, i, hi, far_first);
            hi = j;
        }
    }
    insertion(recs, keys, lo, hi, far_first);
}

static void sort_impl(instancing_gpu_instance *recs, int count, vec3 cam,
                      int far_first) {
    if (count < 2) return;

    // precompute the distance keys once so the comparator isn't re-reading the
    // matrix every compare. malloc is fine here — sorting only happens for the
    // translucent groups, not every instance every frame.
    float *k = malloc((size_t)count * sizeof(float));
    if (!k) return;  // can't sort without keys; leave order as-is

    for (int i = 0; i < count; ++i)
        k[i] = dist_sq(&recs[i], cam);

    quicksort(recs, k, 0, count - 1, far_first);
    free(k);
}

void instancing_sort_back_to_front(instancing_gpu_instance *recs, int count,
                                   vec3 cam) {
    sort_impl(recs, count, cam, 1);
}

void instancing_sort_front_to_back(instancing_gpu_instance *recs, int count,
                                   vec3 cam) {
    sort_impl(recs, count, cam, 0);
}
