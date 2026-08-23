#ifndef NET_COMPRESS_CONTAINER_H
#define NET_COMPRESS_CONTAINER_H

// the on-wire wrapper around the stage payloads. the encoder runs the chunk
// through palette -> rle -> lz, but bails out of any stage that didnt help
// and records which stages actually ran in `methods`. the decoder replays
// them in reverse.
//
// header:
// u16  magic        (COMPRESS_MAGIC)
// u8   version
// u8   method_count
// u8   methods[method_count]   (in apply order; decode reverses)
// var  raw_count    (original block count, for the buffer size check)
// var  payload_len
// u32  crc32        (over raw blocks, validated after decode)
// ...  payload

#include <stddef.h>
#include <stdint.h>

#include "compress.h"

#define COMPRESS_MAX_STAGES 4

typedef struct {
    uint16_t magic;
    uint8_t  version;
    uint8_t  method_count;
    uint8_t  methods[COMPRESS_MAX_STAGES];
    uint32_t raw_count;
    uint32_t payload_len;
    uint32_t crc32;
} compress_container;

// write everything but the payload bytes. returns header size, 0 on overflow.
size_t compress_container_write_header(uint8_t *out, size_t cap,
                                       const compress_container *c);

// parse just the header. returns header size consumed, 0 if invalid.
size_t compress_container_read_header(const uint8_t *in, size_t len,
                                      compress_container *c);

#endif
