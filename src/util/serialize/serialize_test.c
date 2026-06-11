#include "serialize.h"
#include "serialize_tag.h"
#include "serialize_math.h"
#include "serialize_crc.h"
#include "../log.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
static int g_fail;
#define CHECK(cond, ...) do { \
serialize_writer_init(&w);
serialize_put_u8 (&w, 0xAB);
serialize_put_u16(&w, 0xBEEF);
serialize_put_u32(&w, 0xDEADBEEFu);
serialize_put_u64(&w, 0x0123456789ABCDEFull);
serialize_put_i32(&w, -42);
serialize_put_f32(&w, 3.14159f);
serialize_put_f64(&w, 2.718281828459045);
size_t len;
uint8_t *buf = serialize_writer_take(&w, &len);
CHECK(buf != NULL, "primitive writer errored");
serialize_reader r;
serialize_reader_init(&r, buf, len);
CHECK(serialize_get_u8(&r)  == 0xAB,        "u8 roundtrip");
CHECK(serialize_get_u16(&r) == 0xBEEF,      "u16 roundtrip");
CHECK(serialize_get_u32(&r) == 0xDEADBEEFu, "u32 roundtrip");
CHECK(serialize_get_u64(&r) == 0x0123456789ABCDEFull, "u64 roundtrip");
CHECK(serialize_get_i32(&r) == -42,         "i32 sign roundtrip");
CHECK(fabsf(serialize_get_f32(&r) - 3.14159f) < 1e-6f,  "f32 roundtrip");
CHECK(fabs(serialize_get_f64(&r) - 2.718281828459045) < 1e-12, "f64 roundtrip");
CHECK(r.err == SERIALIZE_OK, "reader clean after primitives");
CHECK(serialize_reader_remaining(&r) == 0, "consumed everything");
free(buf);
serialize_reader r;
serialize_reader_init(&r, small, sizeof small);
serialize_get_u32(&r);
CHECK(r.err == SERIALIZE_ERR_UNDERRUN, "short read should underrun");
CHECK(serialize_get_u8(&r) == 0, "reads after error return zero");
CHECK(r.err == SERIALIZE_ERR_UNDERRUN, "error flag sticks");
serialize_writer_init(&w);
serialize_tag_scope s = serialize_tag_begin(&w, SERIALIZE_FOURCC('T','E','S','T'),
                                                3, SERIALIZE_TAG_F_CRC);
serialize_put_u32(&w, 0xCAFEBABEu);
serialize_put_str(&w, "payload");
serialize_tag_end(&w, &s);
size_t len;
uint8_t *buf = serialize_writer_take(&w, &len);
serialize_reader r;
serialize_reader_init(&r, buf, len);
serialize_tag_hdr h;
CHECK(serialize_tag_next(&r, &h) == 0, "tag header read");
CHECK(h.tag == SERIALIZE_FOURCC('T','E','S','T'), "fourcc roundtrip");
CHECK(h.version == 3, "tag version roundtrip");
CHECK(serialize_get_u32(&r) == 0xCAFEBABEu, "tag body roundtrip");
char tmp[16];
serialize_get_str(&r, tmp, sizeof tmp);
CHECK(strcmp(tmp, "payload") == 0, "tag body str");
CHECK(serialize_tag_skip_body(&r, &h) == 0, "crc validates");
buf[12] ^= 0xFF;
serialize_reader r2;
serialize_reader_init(&r2, buf, len);
serialize_tag_next(&r2, &h);
serialize_skip(&r2, h.body_len);
CHECK(serialize_tag_skip_body(&r2, &h) != 0, "corrupt crc should fail");
free(buf);
