#ifndef WORLD_REGION_TYPES_H
#define WORLD_REGION_TYPES_H
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
// region file format. chunks are grouped 32x32 into a single .vrg file so we
// dont end up with a million tiny files in saves/. layout is the usual
// "minecraft-ish" thing: a fixed header of (offset,length,timestamp) entries
// up front, then a heap of fixed-size sectors that the chunk payloads live in.
//
// [ header table        REGION_HEADER_BYTES ]
// [ sector 0 .. n       REGION_SECTOR_BYTES each ]
//
// header table is REGION_CHUNKS_PER_FILE entries of region_loc_t followed by
// the same count of u32 timestamps. nothing fancy, all little-ish endian since
// we only ever read these back on the same machine anyway.
#define REGION_SIZE_SHIFT     5                 // 32 chunks per axis
#define REGION_SIZE           (1 << REGION_SIZE_SHIFT)
#define REGION_MASK           (REGION_SIZE - 1)
#define REGION_CHUNKS_PER_FILE (REGION_SIZE * REGION_SIZE)   // 1024
// sectors. 4k matches the page size, keeps writes cheap-ish.
#define REGION_SECTOR_BYTES   4096
#define REGION_SECTOR_SHIFT   12
// header: one loc entry (8 bytes) + one timestamp (4 bytes) per chunk slot,
// kept in two separate tables so the loc table stays cache-dense when we scan
// it. rounded up to a whole number of sectors.
#define REGION_LOC_BYTES      (REGION_CHUNKS_PER_FILE * 8)   // 8k
#define REGION_TS_BYTES       (REGION_CHUNKS_PER_FILE * 4)   // 4k
#define REGION_HEADER_BYTES   (REGION_LOC_BYTES + REGION_TS_BYTES)  // 12k
#define REGION_HEADER_SECTORS (REGION_HEADER_BYTES / REGION_SECTOR_BYTES) // 3
#define REGION_MAGIC          0x47525601u       // '\x01VRG'
#define REGION_FMT_VERSION    3
// a single chunk payload may not span more than this many sectors. 255 * 4k =
// ~1mb which is way more than a chunk ever needs even uncompressed, but the
// length field is a byte so this is the hard ceiling.
#define REGION_MAX_SECTORS    255
// compression / encoding tags stored in the per-chunk payload header.
enum {
    REGION_ENC_RAW    = 0,   // blocks[] verbatim, no light
    REGION_ENC_RLE    = 1,   // run length over block ids
    REGION_ENC_PAL    = 2,   // palette + bit-packed indices
}
;
// a header entry. offset is in sectors from the start of the file, count is
// how many sectors the payload occupies. offset==0 && count==0 means the slot
// is empty (offset 0 always points at the header, never a payload, so its a
// safe sentinel).
typedef struct {
    uint32_t offset;    // first sector
    uint8_t  count;     // sector count
    uint8_t  enc;       // REGION_ENC_*
    uint16_t pad;       // keep it 8 bytes, reserved
} region_loc_t;
#endif
