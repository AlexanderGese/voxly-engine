#ifndef WORLD_REGION_COORD_H
#define WORLD_REGION_COORD_H

#include "region_types.h"
#include <stdint.h>

// chunk (cx,cz) -> region (rx,rz) and the local slot index inside that region
// file. arithmetic shift so negative coords floor correctly, the classic
// off-by-one trap if you use plain division.

static inline int region_chunk_to_rx(int cx) { return cx >> REGION_SIZE_SHIFT; }
static inline int region_chunk_to_rz(int cz) { return cz >> REGION_SIZE_SHIFT; }

// local x/z within the region, 0..31
static inline int region_local_x(int cx) { return cx & REGION_MASK; }
static inline int region_local_z(int cz) { return cz & REGION_MASK; }

// slot index into the header table for a chunk. row-major, x fastest.
static inline int region_slot_index(int cx, int cz) {
    return region_local_x(cx) + (region_local_z(cz) << REGION_SIZE_SHIFT);
}

// pack region coords into a u64 key for the open-file cache hashmap. same
// trick world.c uses for chunk keys.
static inline uint64_t region_key(int rx, int rz) {
    return ((uint64_t)(uint32_t)rx) | (((uint64_t)(uint32_t)rz) << 32);
}

// build the on-disk path for a region. caller owns the buffer. dir has no
// trailing slash.
void region_coord_path(char *out, size_t n, const char *dir, int rx, int rz);

// reverse: parse "r.<rx>.<rz>.vrg" back out of a filename. returns 1 on match.
int  region_coord_parse(const char *name, int *rx, int *rz);

#endif
