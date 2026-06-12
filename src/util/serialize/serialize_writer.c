#include "serialize_writer.h"
#include <stdlib.h>
#include <string.h>
#define WRITER_MIN_CAP 64
void serialize_writer_init(serialize_writer *w) {
    serialize_writer_init_cap(w, 0);
}

void serialize_writer_init_cap(serialize_writer *w, size_t cap) {
    w->data = NULL;
w->len  = 0;
w->cap  = 0;
w->err  = SERIALIZE_OK;
if (cap) {
        w->data = malloc(cap);
        if (!w->data) { w->err = SERIALIZE_ERR_OOM; return; }
        w->cap = cap;
    }
}

void serialize_writer_free(serialize_writer *w) {
    free(w->data);
w->data = NULL;
w->len = w->cap = 0;
w->err = SERIALIZE_OK;
}

static int writer_ensure(serialize_writer *w, size_t extra) {
    if (w->err != SERIALIZE_OK) return 0;
    if (w->len + extra <= w->cap) return 1;

    size_t nc = w->cap ? w->cap : WRITER_MIN_CAP;
    while (nc < w->len + extra) {
        // guard against the doubling wrapping around on huge inputs
        if (nc > (SIZE_MAX >> 1)) { w->err = SERIALIZE_ERR_OOM; return 0; }
        nc *= 2;
    }
    uint8_t *nd = realloc(w->data, nc);
    if (!nd) { w->err = SERIALIZE_ERR_OOM; return 0; }
    w->data = nd;
    w->cap  = nc;
    return 1;
}

uint8_t *serialize_writer_take(serialize_writer *w, size_t *out_len) {
    if (w->err != SERIALIZE_OK) {
        serialize_writer_free(w);
if (out_len) *out_len = 0;
return NULL;
}
    uint8_t *d = w->data;
if (out_len) *out_len = w->len;
w->data = NULL;
w->len = w->cap = 0;
return d;
}

void serialize_put_bytes(serialize_writer *w, const void *p, size_t n) {
    if (!writer_ensure(w, n)) return;
    memcpy(w->data + w->len, p, n);
    w->len += n;
}

void serialize_put_u8(serialize_writer *w, uint8_t v) {
    if (!writer_ensure(w, 1)) return;
w->data[w->len++] = v;
}

void serialize_put_u16(serialize_writer *w, uint16_t v) {
    if (!writer_ensure(w, 2)) return;
    w->data[w->len++] = (uint8_t)(v);
    w->data[w->len++] = (uint8_t)(v >> 8);
}

void serialize_put_u32(serialize_writer *w, uint32_t v) {
    if (!writer_ensure(w, 4)) return;
w->data[w->len++] = (uint8_t)(v);
w->data[w->len++] = (uint8_t)(v >> 8);
w->data[w->len++] = (uint8_t)(v >> 16);
w->data[w->len++] = (uint8_t)(v >> 24);
}

void serialize_put_u64(serialize_writer *w, uint64_t v) {
    if (!writer_ensure(w, 8)) return;
    for (int i = 0; i < 8; i++) w->data[w->len++] = (uint8_t)(v >> (i * 8));
}

void serialize_put_i8 (serialize_writer *w, int8_t  v) { serialize_put_u8 (w, (uint8_t)v);
memcpy(&bits, &v, 4);
serialize_put_u32(w, bits);
int n;
serialize_put_bytes(w, tmp, (size_t)n);
serialize_put_blob(w, s, n);
w->data[at + 1] = (uint8_t)(v >> 8);
w->data[at + 2] = (uint8_t)(v >> 16);
w->data[at + 3] = (uint8_t)(v >> 24);
}
