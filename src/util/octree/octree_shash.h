#ifndef UTIL_OCTREE_SHASH_H
#define UTIL_OCTREE_SHASH_H

// uniform-grid spatial hash. the octree is great for big sparse worlds and
// raycasts, but for "give me everyone near this entity" with lots of small
// moving things, a flat grid with a fixed cell size is faster and has zero
// rebalancing. this is that.
//
// each item is bucketed by the cell its center falls in. moves are cheap: if
// the center stays in the same cell we touch nothing. neighbor queries scan a
// 3x3x3 (or bigger) block of cells.

#include "octree_types.h"
#include "../hashmap.h"
#include <stdint.h>

// one entry in a cell bucket. buckets are singly linked through `next` indices
// into a shared pool, classic intrusive list, keeps allocs down.
typedef struct {
    uint32_t id;
    uint32_t tag;
    vec3     pos;
    uint64_t cell;     // packed cell coord, so we can detect "didnt move cells"
    int32_t  next;     // next entry in same bucket, -1 = end
} shash_entry;

typedef struct {
    hashmap      cells;      // packed cell coord -> head entry index (+1)
    shash_entry *entries;    // darray of entries, index-stable, free-listed
    int32_t      freelist;   // -1 if none
    hashmap      locate;     // id -> entry index (+1), for O(1) move/remove
    float        cell_size;
    float        inv_cell;
    int32_t      count;
} octree_shash;

void octree_shash_init(octree_shash *h, float cell_size);
void octree_shash_free(octree_shash *h);
void octree_shash_clear(octree_shash *h);

// insert/replace. pos is the point used for bucketing (entity center usually).
void octree_shash_insert(octree_shash *h, uint32_t id, uint32_t tag, vec3 pos);

// move. cheap no-op if the point stays in the same cell.
void octree_shash_move(octree_shash *h, uint32_t id, vec3 pos);

int  octree_shash_remove(octree_shash *h, uint32_t id);

// gather all ids within `radius` of center. appends entries to *out (a darray).
// returns how many were added. exact distance test, not just cell membership.
int  octree_shash_near(const octree_shash *h, vec3 center, float radius,
                       shash_entry **out);

int  octree_shash_count(const octree_shash *h);

#endif
