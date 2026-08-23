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
s->buf[s->pos++] = (uint8_t)(v & 0xff);
s->buf[s->pos++] = (uint8_t)(v >> 8);
v >>= 7;
if (v) b |= 0x80;
if (!ws_room(s, 1)) return;
s->buf[s->pos++] = b;
} while (v);
s->len = len;
s->pos = 0;
s->err = 0;
return s->buf[s->pos++];
uint32_t v = (uint32_t)s->buf[s->pos]
               | ((uint32_t)s->buf[s->pos + 1] << 8)
               | ((uint32_t)s->buf[s->pos + 2] << 16)
               | ((uint32_t)s->buf[s->pos + 3] << 24);
s->pos += 4;
return v;
const uint8_t *p = s->buf + s->pos;
s->pos += n;
return p;
