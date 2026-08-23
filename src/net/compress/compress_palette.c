#include "compress_palette.h"
#include "compress_stream.h"
#include "compress_bits.h"
#include <string.h>
int compress_palette_build(compress_palette *p,
                           const block_id *blocks, size_t count) {
    p->count = 0;
    for (int i = 0; i < COMPRESS_PALETTE_MAX; i++) p->lookup[i] = -1;

    for (size_t i = 0; i < count; i++) {
        block_id id = blocks[i];
        if (p->lookup[id] < 0) {
            // first time we see this id. order of first appearance, which
            // tends to put air/stone early. doesnt matter for correctness.
            p->lookup[id] = (int16_t)p->count;
            p->entry[p->count] = id;
            p->count++;
        }
    }
    return p->count;
}

size_t compress_palette_encode(const block_id *blocks, size_t count,
                               uint8_t *out, size_t cap) {
    compress_palette pal;
int n = compress_palette_build(&pal, blocks, count);
if (n == 0) return 0;
compress_wstream ws;
compress_ws_init(&ws, out, cap);
compress_ws_var(&ws, (uint32_t)count);
compress_ws_u8(&ws, (uint8_t)(n & 0xff));
for (int i = 0;
i < n;
i++) compress_ws_u8(&ws, pal.entry[i]);
if (ws.err) return 0;
int idx_bits = compress_bits_for((size_t)n);
compress_bitw bw;
compress_bitw_init(&bw, out + ws.pos, cap - ws.pos);
for (size_t i = 0;
i < count;
i++) {
        int idx = pal.lookup[blocks[i]];
        compress_bitw_put(&bw, (uint32_t)idx, idx_bits);
    }
    if (bw.overflow) return 0;
size_t bit_bytes = compress_bitw_flush(&bw);
return ws.pos + bit_bytes;
}

size_t compress_palette_decode(const uint8_t *in, size_t len,
                               block_id *out, size_t count) {
    compress_rstream rs;
    compress_rs_init(&rs, in, len);

    uint32_t block_count = compress_rs_var(&rs);
    uint32_t n = compress_rs_u8(&rs);
    if (n == 0) n = 256; // see encode, 256 wrapped to 0
    if (rs.err) return 0;
    if (block_count > count) return 0; // caller buffer too small

    block_id table[COMPRESS_PALETTE_MAX];
    for (uint32_t i = 0; i < n; i++) table[i] = compress_rs_u8(&rs);
    if (rs.err) return 0;

    int idx_bits = compress_bits_for(n);

    compress_bitr br;
    compress_bitr_init(&br, in + rs.pos, len - rs.pos);
    for (uint32_t i = 0; i < block_count; i++) {
        uint32_t idx = compress_bitr_get(&br, idx_bits);
        if (idx >= n) return 0;             // corrupt index
        out[i] = table[idx];
    }
    if (br.underflow) return 0;

    return block_count;
}
