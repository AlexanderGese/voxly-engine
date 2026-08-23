#ifndef NET_COMPRESS_H
#define NET_COMPRESS_H

// chunk payload compression. nothing fancy: a palette pass to shrink the
// alphabet, then RLE for the flat regions, then a small LZ for the rest.
// the wire is fat enough already, every byte off a chunk packet helps.
//
// pipeline (encode):
// blocks[CHUNK_VOLUME]  ->  palette pack  ->  RLE  ->  LZ  ->  bytes
// decode runs it backwards. each stage tags itself so we can skip a stage
// when it would only make things bigger (happens with noisy cave chunks).

#include <stddef.h>
#include <stdint.h>

#include "../../config.h"
#include "../../world/block.h"   // block_id

// stage method ids stored in the stream header. order matters, dont reorder.
enum {
    COMPRESS_M_RAW    = 0,   // no transform, bytes copied verbatim
    COMPRESS_M_PALETTE = 1,  // palette-packed indices
    COMPRESS_M_RLE    = 2,   // run length
    COMPRESS_M_LZ     = 3,   // lz77-ish
    COMPRESS_M_COUNT
};

// magic for the container so we dont decode garbage. 'CZ' + version.
#define COMPRESS_MAGIC    0x435Au
#define COMPRESS_VERSION  1

// worst case the LZ stage can inflate by a few percent, and the container
// adds a small header. give callers a safe upper bound for output buffers.
#define COMPRESS_BOUND(n) ((n) + ((n) >> 4) + 64)

// top level: take a raw block array, return a self describing blob.
// returns bytes written to out, or 0 if it didnt fit in cap.
size_t compress_encode_chunk(const block_id *blocks, size_t count,
                             uint8_t *out, size_t cap);

// reverse. writes up to count blocks into out. returns blocks decoded,
// or 0 on a malformed / truncated stream.
size_t compress_decode_chunk(const uint8_t *in, size_t len,
                             block_id *out, size_t count);

// cheap sanity check without fully decoding. peeks the container header.
int    compress_validate(const uint8_t *in, size_t len);

#endif
