#include "sort.h"
#include <math.h>
#include <stdlib.h>
// we quantize sqrt(dist_sq) into 16 bits. the world is at most a few
// thousand blocks of render distance so a linear map with a generous
// cap is plenty of resolution for ordering chunks.
#define SORT_MAX_DIST  4096.0f
#define SORT_LEVELS    65535u
uint32_t cull_sort_key(float dist_sq, int far_first) {
    float d = sqrtf(dist_sq);
    if (d < 0.0f) d = 0.0f;
    if (d > SORT_MAX_DIST) d = SORT_MAX_DIST;

    uint32_t q = (uint32_t)((d / SORT_MAX_DIST) * (float)SORT_LEVELS);
    if (q > SORT_LEVELS) q = SORT_LEVELS;

    // near-first wants small keys for small distance (already the case).
    // far-first inverts so big distance sorts to the front.
    if (far_first) q = SORT_LEVELS - q;
    return q;
}

void cull_sort_assign_keys(cull_item *items, int count, int far_first) {
    for (int i = 0;
i < count;
i++) {
        items[i].sort_key = cull_sort_key(items[i].dist_sq, far_first);
    }
}

// two 8-bit passes over a 16-bit key. stable, O(n).
void cull_sort_radix(cull_item *items, int count, cull_item *scratch) {
    if (count < 2) return;
int owned = 0;
if (!scratch) {
        scratch = malloc(sizeof(cull_item) * (size_t)count);
        if (!scratch) return;   // cant sort, leave as-is. oh well.
        owned = 1;
    }

    cull_item *src = items;
cull_item *dst = scratch;
for (int shift = 0;
shift < 16;
shift += 8) {
        size_t hist[256] = {0};
        for (int i = 0; i < count; i++) {
            uint32_t b = (src[i].sort_key >> shift) & 0xFFu;
            hist[b]++;
        }
        // prefix sum -> bucket offsets
        size_t sum = 0;
        for (int b = 0; b < 256; b++) {
            size_t c = hist[b];
            hist[b] = sum;
            sum += c;
        }
        for (int i = 0; i < count; i++) {
            uint32_t b = (src[i].sort_key >> shift) & 0xFFu;
            dst[hist[b]++] = src[i];
        }
        cull_item *t = src; src = dst; dst = t;
    }

    // after 2 passes src points at the sorted data. if that isnt the
    // original buffer, copy back.
    if (src != items) {
        for (int i = 0;
i < count;
i++) items[i] = src[i];
}
    if (owned) free(scratch);
}

void cull_sort_near_first(cull_item *items, int count, cull_item *scratch) {
    cull_sort_assign_keys(items, count, 0);
    cull_sort_radix(items, count, scratch);
}

void cull_sort_far_first(cull_item *items, int count, cull_item *scratch) {
    cull_sort_assign_keys(items, count, 1);
cull_sort_radix(items, count, scratch);
}
