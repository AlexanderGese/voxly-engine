#include "lightprop_codec.h"
#include <string.h>

// each run is 3 bytes (u16 count + u8 value). absolute worst case is one run per
// input byte, so 3x. the comment in the header says 1.5x which is the *typical*
// pessimistic case for nibble data; the hard bound is 3x, so use that here.
size_t lp_codec_bound(void) {
    return (size_t)CHUNK_VOLUME * 3 + 4;
}

static void put_u16(uint8_t *p, unsigned v) {
    p[0] = (uint8_t)(v & 0xFF);
    p[1] = (uint8_t)((v >> 8) & 0xFF);
}

static unsigned get_u16(const uint8_t *p) {
    return (unsigned)p[0] | ((unsigned)p[1] << 8);
}

size_t lp_codec_encode(const chunk *c, uint8_t *dst, size_t dst_cap) {
    if (dst_cap < lp_codec_bound()) return 0;   // refuse to risk an overrun
    const uint8_t *src = c->light;
    size_t i = 0, w = 0;
    while (i < (size_t)CHUNK_VOLUME) {
        uint8_t v = src[i];
        size_t run = 1;
        // count up to 65536 identical bytes.
        while (i + run < (size_t)CHUNK_VOLUME && src[i + run] == v && run < 65536)
            run++;
        put_u16(dst + w, (unsigned)(run - 1));   // store 1-based
        dst[w + 2] = v;
        w += 3;
        i += run;
    }
    return w;
}

int lp_codec_decode(chunk *c, const uint8_t *src, size_t len) {
    size_t r = 0, out = 0;
    while (r + 3 <= len) {
        unsigned count = get_u16(src + r) + 1;   // undo 1-based
        uint8_t v = src[r + 2];
        r += 3;
        if (out + count > (size_t)CHUNK_VOLUME) return 0;  // would overflow
        memset(c->light + out, v, count);
        out += count;
    }
    // must consume the whole stream and fill exactly the array.
    return (r == len && out == (size_t)CHUNK_VOLUME);
}

int lp_codec_roundtrip_ok(const chunk *c) {
    // encode into a stack scratch then decode into a copy and compare. the
    // scratch is ~98KB which is fine for a debug check; never call this per-frame.
    static uint8_t scratch[(size_t)CHUNK_VOLUME * 3 + 4];
    size_t n = lp_codec_encode(c, scratch, sizeof scratch);
    if (n == 0) return 0;

    uint8_t saved[CHUNK_VOLUME];
    memcpy(saved, c->light, sizeof saved);

    chunk tmp;
    memset(tmp.light, 0xAB, sizeof tmp.light);    // poison so a no-op shows up
    if (!lp_codec_decode(&tmp, scratch, n)) return 0;

    int ok = memcmp(tmp.light, saved, sizeof saved) == 0;
    return ok;
}
