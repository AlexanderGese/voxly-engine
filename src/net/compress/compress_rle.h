#ifndef NET_COMPRESS_RLE_H
#define NET_COMPRESS_RLE_H

// run length pass over a byte buffer. terrain is mostly long vertical runs of
// the same id (air column, stone column) so this alone wins big on the lower
// and upper slices of a chunk.
//
// token format, repeated:
// var  run_length    (>= 1)
// u8   value
//
// runs are capped so a single run length stays a sane varint; the encoder
// splits longer runs. trivial but it keeps the decoder bounds simple.

#include <stddef.h>
#include <stdint.h>

// longest run we emit in a single token. 64k is plenty for a chunk slice.
#define COMPRESS_RLE_MAX_RUN 0xFFFFu

size_t compress_rle_encode(const uint8_t *in, size_t in_len,
                           uint8_t *out, size_t cap);

size_t compress_rle_decode(const uint8_t *in, size_t in_len,
                           uint8_t *out, size_t cap);

// estimate the encoded size without writing. used to decide if RLE is worth
// it before committing the bytes.
size_t compress_rle_estimate(const uint8_t *in, size_t in_len);

#endif
