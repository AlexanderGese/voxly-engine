#ifndef WORLD_REGION_FILE_H
#define WORLD_REGION_FILE_H

#include "region_types.h"
#include "region_header.h"
#include "region_sector.h"
#include "../chunk.h"
#include <stdio.h>

// one open region file. holds the FILE handle, the resident header table and
// the sector allocator rebuilt from it. read/write chunk go through here.
//
// not thread safe. the world loader serializes region io on a single worker so
// thats fine for us.

typedef struct {
    int             rx, rz;
    FILE           *f;
    region_header_t header;
    region_alloc_t  alloc;
    int             writable;       // opened "r+b" vs "rb"
} region_file_t;

// open (or create) the region file covering region coords (rx,rz) inside dir.
// create==1 will make a fresh file with an empty header if none exists; if 0
// and the file is missing this returns <0 and leaves *rf untouched.
int  region_file_open(region_file_t *rf, const char *dir, int rx, int rz, int create);

// flush header if dirty and close the handle.
void region_file_close(region_file_t *rf);

// is the chunk at (cx,cz) present in this file? (cx,cz) must fall in (rx,rz).
int  region_file_has_chunk(const region_file_t *rf, int cx, int cz);

// load chunk (cx,cz) into c. returns 1 if loaded, 0 if absent, <0 on error.
int  region_file_read_chunk(region_file_t *rf, chunk *c);

// store chunk c. allocates/relocates sectors, rewrites the header slot. returns
// 0 ok, <0 on error. include_light keeps the lighting arrays in the payload.
int  region_file_write_chunk(region_file_t *rf, const chunk *c, int include_light);

// drop a chunk, freeing its sectors. returns 0 ok.
int  region_file_delete_chunk(region_file_t *rf, int cx, int cz);

// unix timestamp of last write for a chunk, or 0 if absent.
uint32_t region_file_chunk_mtime(const region_file_t *rf, int cx, int cz);

#endif
