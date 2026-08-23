#ifndef NET_COMPRESS_BITS_H
#define NET_COMPRESS_BITS_H

// little bit writer / reader pair. msb-first within a byte so the dumps are
// readable when you stare at them in a hex editor (you will, trust me).
// used by the palette stage to pack sub-byte indices.

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t *buf;
    size_t   cap;        // bytes
    size_t   byte;       // current byte index
    int      bit;        // 0..7, next bit to write within buf[byte]
    int      overflow;   // set once we run past cap
} compress_bitw;

typedef struct {
    const uint8_t *buf;
    size_t   len;        // bytes
    size_t   byte;
    int      bit;
    int      underflow;  // read past the end
} compress_bitr;

void     compress_bitw_init(compress_bitw *w, uint8_t *buf, size_t cap);
void     compress_bitw_put(compress_bitw *w, uint32_t value, int nbits);
size_t   compress_bitw_flush(compress_bitw *w);   // bytes used, pads last byte

void     compress_bitr_init(compress_bitr *r, const uint8_t *buf, size_t len);
uint32_t compress_bitr_get(compress_bitr *r, int nbits);
int      compress_bitr_eof(const compress_bitr *r);

// how many bits to address `n` distinct symbols. 0 -> 0, 1 -> 0 (degenerate),
// 2 -> 1, 5 -> 3, etc. callers special-case the <=1 cases.
int      compress_bits_for(size_t n);

#endif
