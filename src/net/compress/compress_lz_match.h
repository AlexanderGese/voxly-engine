#ifndef NET_COMPRESS_LZ_MATCH_H
#define NET_COMPRESS_LZ_MATCH_H
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
#endif
