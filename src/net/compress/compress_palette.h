#ifndef NET_COMPRESS_PALETTE_H
#define NET_COMPRESS_PALETTE_H

// palette stage. a chunk usually touches a handful of block ids (stone, dirt,
// air, maybe an ore or two). instead of a byte per block we build a palette
// of the distinct ids and emit indices packed to ceil(log2(n)) bits each.
//
// payload layout:
// var   block_count        (original element count)
// u8    palette_size        (1..256, 0 is invalid)
// u8    palette[palette_size]
// bits  indices[block_count]  packed, msb-first, byte aligned at the end
//
// degenerate case: a fully uniform chunk has palette_size 1 and zero index
// bits, so it costs basically the header. air-only chunks are free.

#include <stddef.h>
#include <stdint.h>

#include "../../config.h"
#include "../../world/block.h"   // block_id

// block ids are uint8_t so the palette can be at most 256 entries.
#define COMPRESS_PALETTE_MAX 256

typedef struct {
    block_id entry[COMPRESS_PALETTE_MAX];
    int      count;
    int16_t  lookup[COMPRESS_PALETTE_MAX]; // block_id -> index, -1 if absent
} compress_palette;

// scan blocks, fill the palette. returns the distinct id count.
int    compress_palette_build(compress_palette *p,
                              const block_id *blocks, size_t count);

// encode blocks into out using the palette layout. returns bytes written,
// or 0 on overflow.
size_t compress_palette_encode(const block_id *blocks, size_t count,
                               uint8_t *out, size_t cap);

// decode. writes up to count blocks, returns the number actually produced.
size_t compress_palette_decode(const uint8_t *in, size_t len,
                               block_id *out, size_t count);

#endif
