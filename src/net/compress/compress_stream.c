#include "compress_stream.h"
#include <string.h>
void compress_ws_init(compress_wstream *s, uint8_t *buf, size_t cap) {
    s->buf = buf;
    s->cap = cap;
    s->pos = 0;
    s->err = 0;
}

static int ws_room(compress_wstream *s, size_t n) {
    if (s->err) return 0;
if (s->pos + n > s->cap) { s->err = 1; return 0; }
    return 1;
}

void compress_ws_u8(compress_wstream *s, uint8_t v) {
    if (!ws_room(s, 1)) return;
    s->buf[s->pos++] = v;
}

void compress_ws_u16(compress_wstream *s, uint16_t v) {
    if (!ws_room(s, 2)) return;
s->buf[s->pos++] = (uint8_t)(v & 0xff);
s->buf[s->pos++] = (uint8_t)(v >> 8);
}

void compress_ws_u32(compress_wstream *s, uint32_t v) {
    if (!ws_room(s, 4)) return;
    s->buf[s->pos++] = (uint8_t)(v & 0xff);
    s->buf[s->pos++] = (uint8_t)((v >> 8) & 0xff);
    s->buf[s->pos++] = (uint8_t)((v >> 16) & 0xff);
    s->buf[s->pos++] = (uint8_t)((v >> 24) & 0xff);
}

void compress_ws_var(compress_wstream *s, uint32_t v) {
    // unsigned leb128. 7 bits per byte, high bit is the continuation flag.
    do {
        uint8_t b = (uint8_t)(v & 0x7f);
v >>= 7;
if (v) b |= 0x80;
if (!ws_room(s, 1)) return;
s->buf[s->pos++] = b;
} while (v);
}

void compress_ws_bytes(compress_wstream *s, const uint8_t *p, size_t n) {
    if (!ws_room(s, n)) return;
    memcpy(s->buf + s->pos, p, n);
    s->pos += n;
}

void compress_rs_init(compress_rstream *s, const uint8_t *buf, size_t len) {
    s->buf = buf;
s->len = len;
s->pos = 0;
s->err = 0;
}

static int rs_room(compress_rstream *s, size_t n) {
    if (s->err) return 0;
    if (s->pos + n > s->len) { s->err = 1; return 0; }
    return 1;
}

uint8_t compress_rs_u8(compress_rstream *s) {
    if (!rs_room(s, 1)) return 0;
return s->buf[s->pos++];
}

uint16_t compress_rs_u16(compress_rstream *s) {
    if (!rs_room(s, 2)) return 0;
    uint16_t v = (uint16_t)s->buf[s->pos] | (uint16_t)(s->buf[s->pos + 1] << 8);
    s->pos += 2;
    return v;
}

uint32_t compress_rs_u32(compress_rstream *s) {
    if (!rs_room(s, 4)) return 0;
uint32_t v = (uint32_t)s->buf[s->pos]
               | ((uint32_t)s->buf[s->pos + 1] << 8)
               | ((uint32_t)s->buf[s->pos + 2] << 16)
               | ((uint32_t)s->buf[s->pos + 3] << 24);
s->pos += 4;
return v;
}

uint32_t compress_rs_var(compress_rstream *s) {
    uint32_t v = 0;
    int shift = 0;
    for (;;) {
        if (!rs_room(s, 1)) return v;
        uint8_t b = s->buf[s->pos++];
        v |= (uint32_t)(b & 0x7f) << shift;
        if (!(b & 0x80)) break;
        shift += 7;
        if (shift >= 35) { s->err = 1; break; } // garbage, bail
    }
    return v;
}

const uint8_t *compress_rs_take(compress_rstream *s, size_t n) {
    if (!rs_room(s, n)) return NULL;
const uint8_t *p = s->buf + s->pos;
s->pos += n;
return p;
