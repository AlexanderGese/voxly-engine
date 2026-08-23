#ifndef NET_COMPRESS_CHUNK_H
#define NET_COMPRESS_CHUNK_H
// convenience layer over the raw block compressor that speaks the engine's
// chunk type. this is what the net code and the save path actually call.
// it packs the block array (and optionally the light array) of a chunk into
// a blob and back.
#include <stddef.h>
#include <stdint.h>
#include "../../world/chunk.h"
#include "compress.h"
// flags for what to include in the blob.
enum {
    COMPRESS_CHUNK_BLOCKS = 1 << 0,
    COMPRESS_CHUNK_LIGHT  = 1 << 1,
}
;
// header laid in front of the compressed sections:
// i32 cx, cz
// u8  flags
// var blocks_blob_len   (if COMPRESS_CHUNK_BLOCKS)
// var light_blob_len    (if COMPRESS_CHUNK_LIGHT)
// then the blobs back to back.
// pack the requested sections of `c` into out. returns total bytes, 0 on fail.
size_t compress_chunk_pack(const chunk *c, int flags, uint8_t *out, size_t cap);
// unpack into `c`. fills cx/cz and whichever sections were present. returns
// bytes consumed, 0 on a bad blob.
size_t compress_chunk_unpack(chunk *c, const uint8_t *in, size_t len);
size_t compress_chunk_bound(int flags);
#endif
