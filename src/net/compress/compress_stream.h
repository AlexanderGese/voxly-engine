#ifndef NET_COMPRESS_STREAM_H
#define NET_COMPRESS_STREAM_H

// byte oriented cursor used to lay out the container and the stage payloads.
// everything is little endian on the wire. varints for counts so small
// chunks dont pay for 32 bit lengths.

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t *buf;
    size_t   cap;
    size_t   pos;
    int      err;     // set on overflow, stays set
} compress_wstream;

typedef struct {
    const uint8_t *buf;
    size_t   len;
    size_t   pos;
    int      err;     // set on underflow
} compress_rstream;

void compress_ws_init(compress_wstream *s, uint8_t *buf, size_t cap);
void compress_ws_u8 (compress_wstream *s, uint8_t v);
void compress_ws_u16(compress_wstream *s, uint16_t v);
void compress_ws_u32(compress_wstream *s, uint32_t v);
void compress_ws_var(compress_wstream *s, uint32_t v);          // leb128
void compress_ws_bytes(compress_wstream *s, const uint8_t *p, size_t n);

void     compress_rs_init(compress_rstream *s, const uint8_t *buf, size_t len);
uint8_t  compress_rs_u8 (compress_rstream *s);
uint16_t compress_rs_u16(compress_rstream *s);
uint32_t compress_rs_u32(compress_rstream *s);
uint32_t compress_rs_var(compress_rstream *s);
const uint8_t *compress_rs_take(compress_rstream *s, size_t n); // borrow n bytes

// bytes a varint would occupy. handy when sizing a header up front.
size_t   compress_var_size(uint32_t v);

#endif
