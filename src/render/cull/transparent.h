#ifndef RENDER_CULL_TRANSPARENT_H
#define RENDER_CULL_TRANSPARENT_H

#include "cull_types.h"

// transparent draw ordering. water and glass chunks have to be drawn after
// the opaque pass, back-to-front, with depth writes off, or the blending
// looks wrong (you'd see water occluding water behind it).
//
// the opaque cull already produced a near->far visible list. for the
// transparent pass we want the same survivors but reversed-ish (far->near)
// AND only the chunks that actually have transparent geometry. we dont
// have per-chunk material splits here, so we approximate: any visible
// chunk is a candidate, the renderer's transparent shader pass just no-ops
// on chunks with no transparent verts.

typedef struct {
    cull_item *items;   // not owned — points into a caller buffer
    int        count;
} cull_transparent_list;

// fill 'out' (capacity cap) with the visible items in far->near order.
// 'src' is the opaque survivor list (near->far). returns the count written.
// out may NOT alias src.
int cull_transparent_build(const cull_item *src, int src_count,
                           cull_item *out, int cap);

// in-place reverse of an item range. handy if you already have the list
// in near->far order and just want to flip it without a second buffer.
void cull_transparent_reverse(cull_item *items, int count);

// sum up the (rough) transparent draw count, for stats. right now this is
// just the item count since we cant tell transparent verts apart, but it
// gives the overlay a number to show.
int cull_transparent_estimate(const cull_item *items, int count);

#endif
