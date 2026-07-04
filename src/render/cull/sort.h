#ifndef RENDER_CULL_SORT_H
#define RENDER_CULL_SORT_H

#include "cull_types.h"

// draw-order sorting for the visible set.
//
// two orders matter:
// - opaque chunks: front-to-back, so early-z throws away occluded
// fragments (and so the occlusion pass works at all).
// - transparent chunks (water, glass): back-to-front for blending.
//
// distances are quantized into a 16-bit key and radix sorted. could just
// qsort but radix is stable and branch-light and i had the buckets lying
// around from the chunk queue work.

// build the packed sort key for an item from its dist_sq. far_first flips
// it for the transparent pass.
uint32_t cull_sort_key(float dist_sq, int far_first);

// assign keys to every item for a front-to-back opaque pass.
void cull_sort_assign_keys(cull_item *items, int count, int far_first);

// stable LSB-first radix sort on the precomputed 16-bit sort_key.
// scratch must hold >= count items. if scratch is NULL it mallocs one.
void cull_sort_radix(cull_item *items, int count, cull_item *scratch);

// convenience wrappers.
void cull_sort_near_first(cull_item *items, int count, cull_item *scratch);
void cull_sort_far_first (cull_item *items, int count, cull_item *scratch);

#endif
