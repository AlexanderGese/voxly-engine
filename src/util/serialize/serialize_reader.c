#include "serialize_reader.h"

#include <string.h>

void serialize_reader_init(serialize_reader *r, const void *data, size_t len) {
    r->base = (const uint8_t *)data;
    r->p    = (const uint8_t *)data;
    r->left = len;
    r->err  = SERIALIZE_OK;
}

size_t serialize_reader_tell(const serialize_reader *r) {
    return (size_t)(r->p - r->base);
}

size_t serialize_reader_remaining(const serialize_reader *r) {
    return r->left;
}

// the one place that checks bounds. everything else funnels through here.
static const uint8_t *reader_take(serialize_reader *r, size_t n) {
    if (r->err != SERIALIZE_OK) return NULL;
    if (n > r->left) { r->err = SERIALIZE_ERR_UNDERRUN; return NULL; }
    const uint8_t *at = r->p;
    r->p    += n;
    r->left -= n;
    return at;
}

int serialize_get_bytes(serialize_reader *r, void *out, size_t n) {
    const uint8_t *at = reader_take(r, n);
    if (!at) return -1;
    memcpy(out, at, n);
    return 0;
}

const void *serialize_get_borrow(serialize_reader *r, size_t n) {
    return reader_take(r, n);
}

void serialize_skip(serialize_reader *r, size_t n) {
    reader_take(r, n);
}

uint8_t serialize_get_u8(serialize_reader *r) {
    const uint8_t *at = reader_take(r, 1);
    return at ? at[0] : 0;
}

uint16_t serialize_get_u16(serialize_reader *r) {
    const uint8_t *at = reader_take(r, 2);
    if (!at) return 0;
    return (uint16_t)at[0] | ((uint16_t)at[1] << 8);
}

uint32_t serialize_get_u32(serialize_reader *r) {
    const uint8_t *at = reader_take(r, 4);
    if (!at) return 0;
    return  (uint32_t)at[0]        | ((uint32_t)at[1] << 8) |
           ((uint32_t)at[2] << 16) | ((uint32_t)at[3] << 24);
}

uint64_t serialize_get_u64(serialize_reader *r) {
    const uint8_t *at = reader_take(r, 8);
    if (!at) return 0;
    uint64_t v = 0;
    for (int i = 0; i < 8; i++) v |= (uint64_t)at[i] << (i * 8);
    return v;
}

int8_t  serialize_get_i8 (serialize_reader *r) { return (int8_t) serialize_get_u8 (r); }
int16_t serialize_get_i16(serialize_reader *r) { return (int16_t)serialize_get_u16(r); }
int32_t serialize_get_i32(serialize_reader *r) { return (int32_t)serialize_get_u32(r); }
int64_t serialize_get_i64(serialize_reader *r) { return (int64_t)serialize_get_u64(r); }

float serialize_get_f32(serialize_reader *r) {
    uint32_t bits = serialize_get_u32(r);
    float f;
    memcpy(&f, &bits, 4);
    return f;
}

double serialize_get_f64(serialize_reader *r) {
    uint64_t bits = serialize_get_u64(r);
    double d;
    memcpy(&d, &bits, 8);
    return d;
}

uint64_t serialize_get_varint(serialize_reader *r) {
    uint64_t v = 0;
    int shift = 0;
    for (;;) {
        const uint8_t *at = reader_take(r, 1);
        if (!at) return 0;
        uint8_t b = *at;
        // 10 groups of 7 bits is the max a u64 can hold. anything past that
        // is a malformed stream, not a value we lost precision on.
        if (shift >= 64 || (shift == 63 && (b & 0x7E))) {
            r->err = SERIALIZE_ERR_OVERFLOW;
            return 0;
        }
        v |= (uint64_t)(b & 0x7F) << shift;
        if (!(b & 0x80)) break;
        shift += 7;
    }
    return v;
}

const void *serialize_get_blob(serialize_reader *r, size_t *out_len) {
    uint64_t n = serialize_get_varint(r);
    if (r->err != SERIALIZE_OK) { if (out_len) *out_len = 0; return NULL; }
    if (n > r->left) { r->err = SERIALIZE_ERR_UNDERRUN; if (out_len) *out_len = 0; return NULL; }
    const void *p = reader_take(r, (size_t)n);
    if (out_len) *out_len = (size_t)n;
    return p;
}

size_t serialize_get_str(serialize_reader *r, char *out, size_t cap) {
    size_t n = 0;
    const char *s = serialize_get_blob(r, &n);
    if (!s || cap == 0) { if (cap) out[0] = 0; return n; }
    size_t copy = n < cap - 1 ? n : cap - 1;
    memcpy(out, s, copy);
    out[copy] = 0;
    return n;
}
