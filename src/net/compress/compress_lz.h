#ifndef NET_COMPRESS_LZ_H
#define NET_COMPRESS_LZ_H

// a small lz77 variant. greedy parse, hash-chain match finder over a fixed
// window. tokens are byte-tagged: a control byte holds 8 flags, each flag
// says whether the next item is a literal or a back-reference. classic
// lz-style layout, nothing clever, but it mops up the repetition RLE leaves
// behind (alternating patterns, structured ore veins, etc).
//
// reference encoding:
// var  distance   (1..window)
// var  length     (MIN_MATCH..MAX_MATCH)
// literal: a single raw byte.

#include <stddef.h>
#include <stdint.h>

// window must be a power of two so the hash mask is cheap.
#define COMPRESS_LZ_WINDOW    4096
#define COMPRESS_LZ_HASH_BITS 13
#define COMPRESS_LZ_HASH_SIZE (1u << COMPRESS_LZ_HASH_BITS)

#define COMPRESS_LZ_MIN_MATCH 3
#define COMPRESS_LZ_MAX_MATCH 273      // min + 270, fits a 2-byte varint nicely
#define COMPRESS_LZ_MAX_CHAIN 32       // how far we walk a hash chain

size_t compress_lz_encode(const uint8_t *in, size_t in_len,
                          uint8_t *out, size_t cap);

size_t compress_lz_decode(const uint8_t *in, size_t in_len,
                          uint8_t *out, size_t cap);

#endif
