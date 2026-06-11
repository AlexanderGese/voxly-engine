#ifndef UTIL_SERIALIZE_READER_H
#define UTIL_SERIALIZE_READER_H

#include <stddef.h>
#include <stdint.h>

#include "serialize.h"

// cursor over a const byte span. does not own the memory. mirror of the
// writer: every get is little-endian, and once err is set every get returns
// a zero and consumes nothing. so a decode can blast through the whole struct
// and only check r->err at the bottom.

typedef struct {
    const uint8_t *base;   // start, for tell()
    const uint8_t *p;      // cursor
    size_t         left;   // bytes remaining at p
    serialize_err  err;
} serialize_reader;

void serialize_reader_init(serialize_reader *r, const void *data, size_t len);

// how many bytes consumed / remaining so far.
size_t serialize_reader_tell(const serialize_reader *r);
size_t serialize_reader_remaining(const serialize_reader *r);

// raw copy. returns 0 on success, sets UNDERRUN otherwise.
int serialize_get_bytes(serialize_reader *r, void *out, size_t n);

// borrow n bytes in place without copying. returns pointer into the span or
// NULL on underrun. handy for big block arrays you dont want to double-buffer.
const void *serialize_get_borrow(serialize_reader *r, size_t n);

// skip ahead without reading. for forward-compat: jump over a section you
// dont understand.
void serialize_skip(serialize_reader *r, size_t n);

uint8_t  serialize_get_u8 (serialize_reader *r);
uint16_t serialize_get_u16(serialize_reader *r);
uint32_t serialize_get_u32(serialize_reader *r);
uint64_t serialize_get_u64(serialize_reader *r);
int8_t   serialize_get_i8 (serialize_reader *r);
int16_t  serialize_get_i16(serialize_reader *r);
int32_t  serialize_get_i32(serialize_reader *r);
int64_t  serialize_get_i64(serialize_reader *r);

float    serialize_get_f32(serialize_reader *r);
double   serialize_get_f64(serialize_reader *r);

uint64_t serialize_get_varint(serialize_reader *r);

// reads a varint length, then borrows that many bytes in place.
// *out_len gets the length. returns the pointer (NULL on error).
const void *serialize_get_blob(serialize_reader *r, size_t *out_len);

// reads a length-prefixed string into out (always nul-terminated, truncated
// to cap-1). returns the on-wire length (which may exceed what was copied).
size_t serialize_get_str(serialize_reader *r, char *out, size_t cap);

#endif
