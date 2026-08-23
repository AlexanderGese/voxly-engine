#include "compress_bits.h"

void compress_bitw_init(compress_bitw *w, uint8_t *buf, size_t cap) {
    w->buf = buf;
    w->cap = cap;
    w->byte = 0;
    w->bit = 0;
    w->overflow = 0;
    if (cap > 0) buf[0] = 0;
}

void compress_bitw_put(compress_bitw *w, uint32_t value, int nbits) {
    if (nbits <= 0) return;
    if (nbits > 32) nbits = 32;

    // walk from the high bit down so the stream is msb-first.
    for (int i = nbits - 1; i >= 0; i--) {
        if (w->byte >= w->cap) {
            w->overflow = 1;
            return;
        }
        int b = (value >> i) & 1u;
        if (b) w->buf[w->byte] |= (uint8_t)(1u << (7 - w->bit));

        if (++w->bit == 8) {
            w->bit = 0;
            if (++w->byte < w->cap) w->buf[w->byte] = 0;
        }
    }
}

size_t compress_bitw_flush(compress_bitw *w) {
    // a partially filled byte still counts. the trailing bits are already
    // zero because we clear each byte on entry.
    if (w->bit != 0) {
        size_t used = w->byte + 1;
        return used <= w->cap ? used : w->cap;
    }
    return w->byte <= w->cap ? w->byte : w->cap;
}

void compress_bitr_init(compress_bitr *r, const uint8_t *buf, size_t len) {
    r->buf = buf;
    r->len = len;
    r->byte = 0;
    r->bit = 0;
    r->underflow = 0;
}

uint32_t compress_bitr_get(compress_bitr *r, int nbits) {
    if (nbits <= 0) return 0;
    if (nbits > 32) nbits = 32;

    uint32_t out = 0;
    for (int i = 0; i < nbits; i++) {
        if (r->byte >= r->len) {
            r->underflow = 1;
            return out << (nbits - i); // best effort, caller should bail
        }
        int b = (r->buf[r->byte] >> (7 - r->bit)) & 1u;
        out = (out << 1) | (uint32_t)b;

        if (++r->bit == 8) {
            r->bit = 0;
            r->byte++;
        }
    }
    return out;
}

int compress_bitr_eof(const compress_bitr *r) {
    return r->byte >= r->len;
}

int compress_bits_for(size_t n) {
    if (n <= 1) return 0;
    int bits = 0;
    size_t v = n - 1;
    while (v) { bits++; v >>= 1; }
    return bits;
}
