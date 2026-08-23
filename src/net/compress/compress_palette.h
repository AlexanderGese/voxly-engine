#ifndef NET_COMPRESS_PALETTE_H
#define NET_COMPRESS_PALETTE_H
// palette stage. a chunk usually touches a handful of block ids (stone, dirt,
// air, maybe an ore or two). instead of a byte per block we build a palette
#include <stddef.h>
#include <stdint.h>
#include "../../config.h"
#include "../../world/block.h"   // block_id
#define COMPRESS_PALETTE_MAX 256
typedef struct {
    block_id entry[COMPRESS_PALETTE_MAX];
    int      count;
    int16_t  lookup[COMPRESS_PALETTE_MAX]; // block_id -> index, -1 if absent
} compress_palette;
int    compress_palette_build(compress_palette *p,
                              const block_id *blocks, size_t count);
size_t compress_palette_encode(const block_id *blocks, size_t count,
                               uint8_t *out, size_t cap);
size_t compress_palette_decode(const uint8_t *in, size_t len,
                               block_id *out, size_t count);
#endif
