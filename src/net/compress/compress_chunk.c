#include "compress_chunk.h"
#include "compress_stream.h"

#include <string.h>

size_t compress_chunk_bound(int flags) {
    size_t n = 16; // header slop: two i32s, flags, two varints
    if (flags & COMPRESS_CHUNK_BLOCKS) n += COMPRESS_BOUND(CHUNK_VOLUME);
    if (flags & COMPRESS_CHUNK_LIGHT)  n += COMPRESS_BOUND(CHUNK_VOLUME);
    return n;
}

size_t compress_chunk_pack(const chunk *c, int flags, uint8_t *out, size_t cap) {
    if (!(flags & (COMPRESS_CHUNK_BLOCKS | COMPRESS_CHUNK_LIGHT))) return 0;

    // we write the header once we know the blob lengths, so compress into a
    // scratch first, then assemble. keeps the varint lengths honest.
    static uint8_t blk[COMPRESS_BOUND(CHUNK_VOLUME)];
    static uint8_t lit[COMPRESS_BOUND(CHUNK_VOLUME)];
    size_t blk_len = 0, lit_len = 0;

    if (flags & COMPRESS_CHUNK_BLOCKS) {
        blk_len = compress_encode_chunk(c->blocks, CHUNK_VOLUME, blk, sizeof blk);
        if (blk_len == 0) return 0;
    }
    if (flags & COMPRESS_CHUNK_LIGHT) {
        // light is a byte per block too, reuse the same block compressor;
        // the nibble-packed values RLE nicely in lit regions.
        lit_len = compress_encode_chunk(c->light, CHUNK_VOLUME, lit, sizeof lit);
        if (lit_len == 0) return 0;
    }

    compress_wstream ws;
    compress_ws_init(&ws, out, cap);
    compress_ws_u32(&ws, (uint32_t)c->cx);
    compress_ws_u32(&ws, (uint32_t)c->cz);
    compress_ws_u8(&ws, (uint8_t)flags);
    if (flags & COMPRESS_CHUNK_BLOCKS) compress_ws_var(&ws, (uint32_t)blk_len);
    if (flags & COMPRESS_CHUNK_LIGHT)  compress_ws_var(&ws, (uint32_t)lit_len);
    if (ws.err) return 0;

    if (flags & COMPRESS_CHUNK_BLOCKS) compress_ws_bytes(&ws, blk, blk_len);
    if (flags & COMPRESS_CHUNK_LIGHT)  compress_ws_bytes(&ws, lit, lit_len);
    if (ws.err) return 0;

    return ws.pos;
}

size_t compress_chunk_unpack(chunk *c, const uint8_t *in, size_t len) {
    compress_rstream rs;
    compress_rs_init(&rs, in, len);

    int32_t cx = (int32_t)compress_rs_u32(&rs);
    int32_t cz = (int32_t)compress_rs_u32(&rs);
    uint8_t flags = compress_rs_u8(&rs);
    if (rs.err) return 0;

    uint32_t blk_len = 0, lit_len = 0;
    if (flags & COMPRESS_CHUNK_BLOCKS) blk_len = compress_rs_var(&rs);
    if (flags & COMPRESS_CHUNK_LIGHT)  lit_len = compress_rs_var(&rs);
    if (rs.err) return 0;

    if (flags & COMPRESS_CHUNK_BLOCKS) {
        const uint8_t *p = compress_rs_take(&rs, blk_len);
        if (!p) return 0;
        size_t got = compress_decode_chunk(p, blk_len, c->blocks, CHUNK_VOLUME);
        if (got != CHUNK_VOLUME) return 0;
    }
    if (flags & COMPRESS_CHUNK_LIGHT) {
        const uint8_t *p = compress_rs_take(&rs, lit_len);
        if (!p) return 0;
        size_t got = compress_decode_chunk(p, lit_len, c->light, CHUNK_VOLUME);
        if (got != CHUNK_VOLUME) return 0;
    }

    c->cx = cx;
    c->cz = cz;
    c->dirty = 1;     // freshly loaded blocks need a remesh
    return rs.pos;
}
