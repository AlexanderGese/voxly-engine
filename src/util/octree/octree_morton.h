#ifndef UTIL_OCTREE_MORTON_H
#define UTIL_OCTREE_MORTON_H

// 3d morton (z-order) codes. used by the spatial hash to give nearby cells
// nearby keys, and by the debug walker to print things in a stable order.
// 21 bits per axis -> fits in a 63 bit code, leaves the top bit alone.

#include <stdint.h>

#define OCTREE_MORTON_BITS   21
#define OCTREE_MORTON_MASK   ((1u << OCTREE_MORTON_BITS) - 1u)

uint64_t octree_morton_encode(uint32_t x, uint32_t y, uint32_t z);
void     octree_morton_decode(uint64_t code, uint32_t *x, uint32_t *y, uint32_t *z);

// child index (0..7) at a given level from a morton code. handy when you want
// to descend the tree straight from a code without recomputing midpoints.
int      octree_morton_child(uint64_t code, int level);

#endif
