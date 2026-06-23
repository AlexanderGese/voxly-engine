#ifndef WORLD_REGION_HEADER_H
#define WORLD_REGION_HEADER_H

#include "region_types.h"
#include <stdint.h>
#include <stdio.h>

// the in-memory mirror of a region file's header table. we read the whole
// thing on open (its only 12k) and keep it resident, flushing back lazily when
// a loc entry changes. timestamps are unix seconds, only used for "which save
// is newer" sanity and the debug overlay.

typedef struct {
    region_loc_t loc[REGION_CHUNKS_PER_FILE];
    uint32_t     ts [REGION_CHUNKS_PER_FILE];
    int          dirty;     // header changed since last flush
} region_header_t;

// zero everything (fresh file).
void region_header_clear(region_header_t *h);

// load header from an already-open file. returns 0 ok, <0 on short read /
// io error. assumes the file pointer can be moved freely.
int  region_header_read(region_header_t *h, FILE *f);

// write the header back to the front of the file. clears the dirty flag.
int  region_header_write(region_header_t *h, FILE *f);

// per-slot accessors. slot is 0..REGION_CHUNKS_PER_FILE-1.
const region_loc_t *region_header_loc(const region_header_t *h, int slot);
uint32_t            region_header_ts (const region_header_t *h, int slot);
int                 region_header_present(const region_header_t *h, int slot);

// stamp a slot with a new payload location + current time, marks dirty.
void region_header_set(region_header_t *h, int slot,
                       uint32_t offset, uint8_t count, uint8_t enc);

// clear a slot (chunk deleted).
void region_header_clear_slot(region_header_t *h, int slot);

#endif
