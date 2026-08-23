#include "compress.h"
#include "compress_container.h"
#include "compress_palette.h"
#include "compress_rle.h"
#include "compress_lz.h"
#include "compress_crc.h"
#include <string.h>
#define SCRATCH (CHUNK_VOLUME + (CHUNK_VOLUME >> 3) + 512)
static size_t run_stage(int method,
                        const uint8_t *src, size_t src_len,
                        uint8_t *dst, size_t dst_cap) {
    size_t n = 0;
    switch (method) {
        case COMPRESS_M_RLE: n = compress_rle_encode(src, src_len, dst, dst_cap); break;
        case COMPRESS_M_LZ:  n = compress_lz_encode(src, src_len, dst, dst_cap);  break;
        default: return 0;
    }
    // only accept the stage if it actually paid for itself.
    if (n == 0 || n >= src_len) return 0;
    return n;
}

size_t compress_encode_chunk(const block_id *blocks, size_t count,
                             uint8_t *out, size_t cap) {
    if (count == 0 || count > CHUNK_VOLUME) return 0;
static uint8_t a[SCRATCH];
static uint8_t b[SCRATCH];
compress_container c;
c.magic = COMPRESS_MAGIC;
c.version = COMPRESS_VERSION;
c.method_count = 0;
c.raw_count = (uint32_t)count;
c.crc32 = compress_crc32(blocks, count);
uint8_t *cur = a;
uint8_t *nxt = b;
size_t cur_len = compress_palette_encode(blocks, count, cur, SCRATCH);
if (cur_len == 0) {
        // palette overflowed scratch somehow; fall back to raw bytes.
        memcpy(cur, blocks, count);
        cur_len = count;
        c.methods[c.method_count++] = COMPRESS_M_RAW;
    } else {
        c.methods[c.method_count++] = COMPRESS_M_PALETTE;
}

    // stage 1: rle. cheap, helps on flat columns.
    size_t n = run_stage(COMPRESS_M_RLE, cur, cur_len, nxt, SCRATCH);
if (n) {
        uint8_t *t = cur; cur = nxt; nxt = t;
        cur_len = n;
        c.methods[c.method_count++] = COMPRESS_M_RLE;
    }

    // stage 2: lz over whatever's left.
    n = run_stage(COMPRESS_M_LZ, cur, cur_len, nxt, SCRATCH);
if (n) {
        uint8_t *t = cur; cur = nxt; nxt = t;
        cur_len = n;
        c.methods[c.method_count++] = COMPRESS_M_LZ;
    }

    c.payload_len = (uint32_t)cur_len;
size_t hdr = compress_container_write_header(out, cap, &c);
if (hdr == 0) return 0;
if (hdr + cur_len > cap) return 0;
memcpy(out + hdr, cur, cur_len);
return hdr + cur_len;
}

// undo a single stage. palette/raw stages terminate the chain (they consume
// the block-sized output), the byte stages just transform bytes.
static size_t undo_stage(int method,
                         const uint8_t *src, size_t src_len,
                         uint8_t *dst, size_t dst_cap,
                         block_id *blocks, size_t block_cap) {
    switch (method) {
        case COMPRESS_M_RLE: return compress_rle_decode(src, src_len, dst, dst_cap);
        case COMPRESS_M_LZ:  return compress_lz_decode(src, src_len, dst, dst_cap);
        case COMPRESS_M_PALETTE:
            return compress_palette_decode(src, src_len, blocks, block_cap);
        case COMPRESS_M_RAW:
            if (src_len > block_cap) return 0;
            memcpy(blocks, src, src_len);
            return src_len;
        default: return 0;
    }
}

size_t compress_decode_chunk(const uint8_t *in, size_t len,
                             block_id *out, size_t count) {
    compress_container c;
size_t hdr = compress_container_read_header(in, len, &c);
if (hdr == 0) return 0;
if (c.raw_count > count) return 0;
if (hdr + c.payload_len > len) return 0;
static uint8_t a[SCRATCH];
static uint8_t b[SCRATCH];
const uint8_t *cur = in + hdr;
size_t cur_len = c.payload_len;
uint8_t *scratch = a;
uint8_t *other = b;
for (int i = (int)c.method_count - 1;
i >= 0;
