#ifndef WORLD_REGION_IO_H
#define WORLD_REGION_IO_H

#include "region_types.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

// thin sector-addressed read/write over a stdio FILE. everything here works in
// sector units so the file_io and sector allocator agree on geometry. the
// payload written to a sector run is prefixed by nothing extra - the codec blob
// already carries its own length - so a partly-filled trailing sector just has
// junk at the end which we never read past total_len.

// read `count` sectors starting at `sector` into buf (must hold count*SECTOR
// bytes). returns 0 ok.
int region_io_read_sectors(FILE *f, uint32_t sector, uint32_t count, void *buf);

// write `len` bytes at `sector`, zero-padding the tail of the last sector so we
// dont leak old data into the file. returns 0 ok.
int region_io_write_payload(FILE *f, uint32_t sector, const void *data, size_t len);

// number of sectors a payload of `len` bytes needs.
uint32_t region_io_sectors_for(size_t len);

// make sure the file is at least `sectors` long, zero-filling any growth. used
// after the allocator bumps `high`.
int region_io_truncate(FILE *f, uint32_t sectors);

// current file length in sectors (rounded down). used to seed the allocator.
uint32_t region_io_file_sectors(FILE *f);

#endif
