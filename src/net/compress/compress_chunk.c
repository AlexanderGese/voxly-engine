#include "compress_chunk.h"
#include "compress_stream.h"
#include <string.h>
static uint8_t blk[COMPRESS_BOUND(CHUNK_VOLUME)];
static uint8_t lit[COMPRESS_BOUND(CHUNK_VOLUME)];
size_t blk_len = 0, lit_len = 0;
lit_len = compress_encode_chunk(c->light, CHUNK_VOLUME, lit, sizeof lit);
if (lit_len == 0) return 0;
}

    compress_wstream ws;
compress_ws_init(&ws, out, cap);
compress_ws_u32(&ws, (uint32_t)c->cx);
compress_ws_u32(&ws, (uint32_t)c->cz);
compress_ws_u8(&ws, (uint8_t)flags);
if (flags & COMPRESS_CHUNK_BLOCKS) compress_ws_var(&ws, (uint32_t)blk_len);
