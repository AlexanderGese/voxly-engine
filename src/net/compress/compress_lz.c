#include "compress_lz.h"
#include "compress_lz_match.h"
#include "compress_stream.h"

// flag layout: a control byte precedes every group of up to 8 items. bit set
// (msb-first) -> the item is a back reference, bit clear -> a literal byte.

size_t compress_lz_encode(const uint8_t *in, size_t in_len,
                          uint8_t *out, size_t cap) {
    if (in_len == 0) return 0;

    static compress_lz_finder finder; // big-ish, keep it out of the stack
    compress_lz_finder_init(&finder, in, in_len);

    compress_wstream ws;
    compress_ws_init(&ws, out, cap);

    size_t pos = 0;
    while (pos < in_len) {
        // reserve the control byte, patch it once the group is full.
        size_t ctrl_at = ws.pos;
        compress_ws_u8(&ws, 0);
        if (ws.err) return 0;
        uint8_t ctrl = 0;

        for (int slot = 0; slot < 8 && pos < in_len; slot++) {
            compress_lz_match m = compress_lz_finder_best(&finder, pos);

            if (m.length >= COMPRESS_LZ_MIN_MATCH) {
                ctrl |= (uint8_t)(1u << (7 - slot));
                compress_ws_var(&ws, (uint32_t)m.distance);
                compress_ws_var(&ws, (uint32_t)(m.length - COMPRESS_LZ_MIN_MATCH));
                // insert every covered position so future matches see them.
                size_t end = pos + m.length;
                while (pos < end) compress_lz_finder_insert(&finder, pos++);
            } else {
                // literal
                compress_ws_u8(&ws, in[pos]);
                compress_lz_finder_insert(&finder, pos);
                pos++;
            }
            if (ws.err) return 0;
        }

        out[ctrl_at] = ctrl;
    }

    return ws.pos;
}

size_t compress_lz_decode(const uint8_t *in, size_t in_len,
                          uint8_t *out, size_t cap) {
    compress_rstream rs;
    compress_rs_init(&rs, in, in_len);

    size_t produced = 0;
    while (rs.pos < rs.len) {
        uint8_t ctrl = compress_rs_u8(&rs);
        if (rs.err) return 0;

        for (int slot = 0; slot < 8 && rs.pos < rs.len; slot++) {
            int is_ref = (ctrl >> (7 - slot)) & 1u;

            if (is_ref) {
                uint32_t dist = compress_rs_var(&rs);
                uint32_t len  = compress_rs_var(&rs) + COMPRESS_LZ_MIN_MATCH;
                if (rs.err) return 0;
                if (dist == 0 || dist > produced) return 0;   // bad reference
                if (produced + len > cap) return 0;

                // overlapping copies are legal (run extension), so go byte
                // by byte rather than memcpy.
                size_t from = produced - dist;
                for (uint32_t k = 0; k < len; k++)
                    out[produced + k] = out[from + k];
                produced += len;
            } else {
                uint8_t b = compress_rs_u8(&rs);
                if (rs.err) return 0;
                if (produced + 1 > cap) return 0;
                out[produced++] = b;
            }
        }
    }
    return produced;
}
