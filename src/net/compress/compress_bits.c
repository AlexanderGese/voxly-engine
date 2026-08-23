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
for (int i = nbits - 1;
i >= 0;
return used <= w->cap ? used : w->cap;
}
    return w->byte <= w->cap ? w->byte : w->cap;
if (nbits > 32) nbits = 32;
uint32_t out = 0;
for (int i = 0;
i < nbits;
int bits = 0;
size_t v = n - 1;
}
