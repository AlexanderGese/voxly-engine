#ifndef WORLD_REGION_SECTOR_H
#define WORLD_REGION_SECTOR_H

#include "region_types.h"
#include <stdint.h>

// sector allocator for one region file. the file is carved into 4k sectors;
// the first REGION_HEADER_SECTORS belong to the header and are permanently
// reserved. everything past that is up for grabs.
//
// we keep a simple in-memory bitmap of used sectors rebuilt from the header
// table at open time. allocation is first-fit which fragments a bit, but chunk
// payloads are tiny and rewritten constantly so it churns flat fast enough. i
// tried a free-list once and it wasnt worth the bookkeeping.

typedef struct {
    uint8_t *bits;      // 1 bit per sector, malloc'd
    uint32_t cap;       // sectors the bitmap can address
    uint32_t high;      // highest allocated sector + 1 (== file length / sector)
} region_alloc_t;

void region_alloc_init(region_alloc_t *a);
void region_alloc_free(region_alloc_t *a);

// mark a run of sectors used / free. used while replaying the header on open.
void region_alloc_mark(region_alloc_t *a, uint32_t start, uint32_t count, int used);

int  region_alloc_is_used(const region_alloc_t *a, uint32_t sector);

// reserve `count` contiguous free sectors, returns the first sector, or 0 if
// it had to grow the file (which it does by bumping `high`). never returns a
// sector inside the header region.
uint32_t region_alloc_reserve(region_alloc_t *a, uint32_t count);

// give a run back to the pool.
void region_alloc_release(region_alloc_t *a, uint32_t start, uint32_t count);

// how many sectors the file should be, for ftruncate/padding.
uint32_t region_alloc_high(const region_alloc_t *a);

#endif
