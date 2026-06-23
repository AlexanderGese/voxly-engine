#ifndef WORLD_REGION_HEADER_H
#define WORLD_REGION_HEADER_H
#include "region_types.h"
#include <stdint.h>
#include <stdio.h>
typedef struct {
    region_loc_t loc[REGION_CHUNKS_PER_FILE];
    uint32_t     ts [REGION_CHUNKS_PER_FILE];
    int          dirty;     // header changed since last flush
} region_header_t;
void region_header_clear(region_header_t *h);
int  region_header_read(region_header_t *h, FILE *f);
int  region_header_write(region_header_t *h, FILE *f);
const region_loc_t *region_header_loc(const region_header_t *h, int slot);
uint32_t            region_header_ts (const region_header_t *h, int slot);
int                 region_header_present(const region_header_t *h, int slot);
#endif
