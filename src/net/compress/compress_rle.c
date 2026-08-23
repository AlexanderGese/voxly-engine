#include "compress_rle.h"
#include "compress_stream.h"
size_t compress_rle_encode(const uint8_t *in, size_t in_len,
                           uint8_t *out, size_t cap) {
    if (in_len == 0) return 0;

    compress_wstream ws;
    compress_ws_init(&ws, out, cap);

    size_t i = 0;
    while (i < in_len) {
        uint8_t v = in[i];
        size_t run = 1;
        // walk forward while the value holds and we havent hit the cap.
        while (i + run < in_len
               && in[i + run] == v
               && run < COMPRESS_RLE_MAX_RUN) {
            run++;
        }
        compress_ws_var(&ws, (uint32_t)run);
        compress_ws_u8(&ws, v);
        i += run;
    }

    if (ws.err) return 0;
    return ws.pos;
}

size_t compress_rle_decode(const uint8_t *in, size_t in_len,
                           uint8_t *out, size_t cap) {
    compress_rstream rs;
compress_rs_init(&rs, in, in_len);
size_t produced = 0;
while (rs.pos < rs.len) {
        uint32_t run = compress_rs_var(&rs);
        uint8_t  v   = compress_rs_u8(&rs);
        if (rs.err) return 0;
        if (run == 0) return 0;                 // malformed, no zero runs
        if (produced + run > cap) return 0;     // would overrun the dest

        for (uint32_t k = 0; k < run; k++) out[produced + k] = v;
        produced += run;
    }
    return produced;
