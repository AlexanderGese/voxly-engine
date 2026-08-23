#ifndef NET_COMPRESS_LZ_MATCH_H
#define NET_COMPRESS_LZ_MATCH_H

// hash-chain match finder for the lz stage. kept separate from the codec so
// the encode loop stays readable. one of these per encode call, lives on the
// stack (its a few tens of kb, fine).

#include <stddef.h>
#include <stdint.h>

#include "compress_lz.h"

typedef struct {
    const uint8_t *src;
    size_t         len;
    // head[hash] = most recent position with that hash, or -1.
    int32_t head[COMPRESS_LZ_HASH_SIZE];
    // prev[pos & mask] = previous position in the same hash chain.
    int32_t prev[COMPRESS_LZ_WINDOW];
} compress_lz_finder;

typedef struct {
    size_t distance;   // 0 means "no match found"
    size_t length;
} compress_lz_match;

void compress_lz_finder_init(compress_lz_finder *f,
                             const uint8_t *src, size_t len);

// record `pos` into the chains so later positions can reference it.
void compress_lz_finder_insert(compress_lz_finder *f, size_t pos);

// find the longest match for the bytes at `pos`. returns a match with
// distance 0 if nothing usable (>= MIN_MATCH) was found.
compress_lz_match compress_lz_finder_best(compress_lz_finder *f, size_t pos);

#endif
