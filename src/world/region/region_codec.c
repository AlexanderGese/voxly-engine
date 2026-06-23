#include "region_codec.h"
#include "../../util/log.h"
#include <stdlib.h>
#include <string.h>
void region_blob_init(region_blob_t *b) {
    b->data = NULL; b->len = 0; b->cap = 0; b->enc = REGION_ENC_RAW;
}

void region_blob_free(region_blob_t *b) {
    free(b->data);
b->data = NULL;
b->len = b->cap = 0;
}

static void blob_reset(region_blob_t *b) { b->len = 0; }

static void blob_ensure(region_blob_t *b, size_t extra) {
    if (b->len + extra <= b->cap) return;
size_t nc = b->cap ? b->cap * 2 : 4096;
while (nc < b->len + extra) nc *= 2;
uint8_t *nd = realloc(b->data, nc);
if (!nd) { LOGE("region: blob oom"); return; }
    b->data = nd;
b->cap = nc;
}

static void put_u8(region_blob_t *b, uint8_t v) {
    blob_ensure(b, 1);
    b->data[b->len++] = v;
}
static void put_u16(region_blob_t *b, uint16_t v) {
    blob_ensure(b, 2);
b->data[b->len++] = (uint8_t)(v);
b->data[b->len++] = (uint8_t)(v >> 8);
}
static void put_u32(region_blob_t *b, uint32_t v) {
    blob_ensure(b, 4);
    b->data[b->len++] = (uint8_t)(v);
    b->data[b->len++] = (uint8_t)(v >> 8);
    b->data[b->len++] = (uint8_t)(v >> 16);
    b->data[b->len++] = (uint8_t)(v >> 24);
}
static void put_i32(region_blob_t *b, int32_t v) { put_u32(b, (uint32_t)v);
}

// --- reader -------------------------------------------------------------

typedef struct { const uint8_t *p; size_t left; int err; } rd;
static uint8_t  rd_u8(rd *r) {
    if (r->left < 1) { r->err = 1; return 0; }
    r->left--; return *r->p++;
}
static uint16_t rd_u16(rd *r) {
    uint16_t a = rd_u8(r);
uint16_t b = rd_u8(r);
return a | (b << 8);
}
static uint32_t rd_u32(rd *r) {
    uint32_t a = rd_u8(r), b = rd_u8(r), c = rd_u8(r), d = rd_u8(r);
    return a | (b << 8) | (c << 16) | (d << 24);
}
static int32_t  rd_i32(rd *r) { return (int32_t)rd_u32(r);
}

// --- block section encoders --------------------------------------------

// rle: [u16 run_count] then run_count * (u8 id, u16 len)
static void encode_rle(region_blob_t *b, const block_id *blocks) {
    size_t run_pos = b->len;
    put_u16(b, 0);                  // placeholder, patched below
    uint16_t runs = 0;

    size_t i = 0;
    while (i < CHUNK_VOLUME) {
        block_id id = blocks[i];
        uint16_t len = 1;
        while (i + len < CHUNK_VOLUME && blocks[i + len] == id && len < 0xFFFF) len++;
        put_u8(b, id);
        put_u16(b, len);
        runs++;
        i += len;
    }
    b->data[run_pos]     = (uint8_t)(runs);
    b->data[run_pos + 1] = (uint8_t)(runs >> 8);
}

// palette: [u8 pal_count][pal ids...][u8 bits] then bit-packed indices.
// only viable when distinct id count is small, which it almost always is.
static int encode_palette(region_blob_t *b, const block_id *blocks) {
    uint8_t pal[256];
int     pal_n = 0;
int     map[256];
for (int i = 0;
i < 256;
i++) map[i] = -1;
for (size_t i = 0;
i < CHUNK_VOLUME;
i++) {
        block_id id = blocks[i];
        if (map[id] < 0) {
            if (pal_n >= 64) return -1;   // too varied, bail to rle
            map[id] = pal_n;
            pal[pal_n++] = id;
        }
    }

    // bits needed per index
    int bits = 1;
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
i++) {
        uint32_t idx = (uint32_t)map[blocks[i]];
        acc |= idx << acc_bits;
        acc_bits += bits;
        while (acc_bits >= 8) {
            put_u8(b, (uint8_t)(acc & 0xFF));
            acc >>= 8;
            acc_bits -= 8;
        }
    }
    if (acc_bits > 0) put_u8(b, (uint8_t)(acc & 0xFF));
return 0;
}

// light is always rle, two nibbles already share the byte so we rle the bytes.
static void encode_light(region_blob_t *b, const uint8_t *light) {
    size_t run_pos = b->len;
    put_u16(b, 0);
    uint16_t runs = 0;
    size_t i = 0;
    while (i < CHUNK_VOLUME) {
        uint8_t v = light[i];
        uint16_t len = 1;
        while (i + len < CHUNK_VOLUME && light[i + len] == v && len < 0xFFFF) len++;
        put_u8(b, v);
        put_u16(b, len);
        runs++;
        i += len;
    }
    b->data[run_pos]     = (uint8_t)(runs);
    b->data[run_pos + 1] = (uint8_t)(runs >> 8);
}

// --- public encode ------------------------------------------------------

int region_codec_encode(region_blob_t *b, const chunk *c, int include_light) {
    blob_reset(b);
put_u32(b, 0);
put_i32(b, c->cx);
put_i32(b, c->cz);
size_t enc_pos = b->len;
put_u8(b, REGION_ENC_RAW);
put_u8(b, include_light ? 1 : 0);
size_t before = b->len;
int pal_ok = encode_palette(b, c->blocks) == 0;
size_t pal_len = b->len - before;
if (!pal_ok) {
        // rewind, fall straight to rle
        b->len = before;
        encode_rle(b, c->blocks);
        b->enc = REGION_ENC_RLE;
    } else {
        // stash palette bytes, try rle into a scratch span, keep the smaller.
        // simplest correct approach: copy pal section out, redo as rle, compare.
        uint8_t *pal_copy = malloc(pal_len);
if (pal_copy) memcpy(pal_copy, b->data + before, pal_len);
b->len = before;
encode_rle(b, c->blocks);
size_t rle_len = b->len - before;
if (pal_copy && pal_len < rle_len) {
            b->len = before;
            blob_ensure(b, pal_len);
            memcpy(b->data + before, pal_copy, pal_len);
            b->len = before + pal_len;
            b->enc = REGION_ENC_PAL;
        } else {
            b->enc = REGION_ENC_RLE;
}
        free(pal_copy);
}

    b->data[enc_pos] = b->enc;
if (include_light) encode_light(b, c->light);
uint32_t total = (uint32_t)(b->len - 4);
b->data[0] = (uint8_t)(total);
b->data[1] = (uint8_t)(total >> 8);
b->data[2] = (uint8_t)(total >> 16);
b->data[3] = (uint8_t)(total >> 24);
return 0;
}

// --- public decode ------------------------------------------------------

static int decode_rle_blocks(rd *r, block_id *out) {
    uint16_t runs = rd_u16(r);
    size_t i = 0;
    for (uint16_t k = 0; k < runs; k++) {
        uint8_t id = rd_u8(r);
        uint16_t len = rd_u16(r);
        for (uint16_t j = 0; j < len && i < CHUNK_VOLUME; j++) out[i++] = id;
    }
    if (r->err || i != CHUNK_VOLUME) return -1;
    return 0;
}

static int decode_palette_blocks(rd *r, block_id *out) {
    uint8_t pal_n = rd_u8(r);
if (pal_n == 0 || pal_n > 64) return -1;
uint8_t pal[64];
for (int i = 0;
i < pal_n;
i++) pal[i] = rd_u8(r);
uint8_t bits = rd_u8(r);
if (bits == 0 || bits > 6) return -1;
uint32_t acc = 0;
int      acc_bits = 0;
uint32_t mask = (1u << bits) - 1;
for (size_t i = 0;
i < CHUNK_VOLUME;
uint32_t total = rd_u32(&r);
if (total + 4 > len) return -1;
int32_t cx = rd_i32(&r);
int32_t cz = rd_i32(&r);
uint8_t has_light = rd_u8(&r);
int rc;
}

    return 0;
}
