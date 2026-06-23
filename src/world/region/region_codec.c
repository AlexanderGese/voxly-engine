#include "region_codec.h"
#include "../../util/log.h"
#include <stdlib.h>
#include <string.h>
b->data = NULL;
b->len = b->cap = 0;
size_t nc = b->cap ? b->cap * 2 : 4096;
while (nc < b->len + extra) nc *= 2;
uint8_t *nd = realloc(b->data, nc);
b->cap = nc;
b->data[b->len++] = (uint8_t)(v);
b->data[b->len++] = (uint8_t)(v >> 8);
}

// --- reader -------------------------------------------------------------

typedef struct { const uint8_t *p; size_t left; int err; } rd;
uint16_t b = rd_u8(r);
return a | (b << 8);
int     pal_n = 0;
int     map[256];
for (int i = 0;
i < 256;
i++) map[i] = -1;
for (size_t i = 0;
i < CHUNK_VOLUME;
while ((1 << bits) < pal_n) bits++;
if (pal_n <= 1) bits = 1;
put_u8(b, (uint8_t)pal_n);
for (int i = 0;
i < pal_n;
i++) put_u8(b, pal[i]);
put_u8(b, (uint8_t)bits);
uint32_t acc = 0;
int      acc_bits = 0;
for (size_t i = 0;
i < CHUNK_VOLUME;
return 0;
put_u32(b, 0);
put_i32(b, c->cx);
put_i32(b, c->cz);
size_t enc_pos = b->len;
put_u8(b, REGION_ENC_RAW);
put_u8(b, include_light ? 1 : 0);
size_t before = b->len;
int pal_ok = encode_palette(b, c->blocks) == 0;
