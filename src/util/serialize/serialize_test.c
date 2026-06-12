#include "serialize.h"
#include "serialize_tag.h"
#include "serialize_math.h"
#include "serialize_crc.h"
#include "../log.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

// self contained roundtrip checks. not wired into a test framework because we
// dont have one; call serialize_run_tests() from a debug menu or main(). it
// LOGs a line per group and returns the number of failures.
//
// the point is to catch the obvious foot-guns: endian drift, varint edges,
// the patch/length bookkeeping, crc, and the sticky-error contract.

static int g_fail;

#define CHECK(cond, ...) do { \
    if (!(cond)) { g_fail++; LOGW("serialize_test: " __VA_ARGS__); } \
} while (0)

static void test_primitives(void) {
    serialize_writer w; serialize_writer_init(&w);
    serialize_put_u8 (&w, 0xAB);
    serialize_put_u16(&w, 0xBEEF);
    serialize_put_u32(&w, 0xDEADBEEFu);
    serialize_put_u64(&w, 0x0123456789ABCDEFull);
    serialize_put_i32(&w, -42);
    serialize_put_f32(&w, 3.14159f);
    serialize_put_f64(&w, 2.718281828459045);

    size_t len; uint8_t *buf = serialize_writer_take(&w, &len);
    CHECK(buf != NULL, "primitive writer errored");

    serialize_reader r; serialize_reader_init(&r, buf, len);
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
}

static void test_varint(void) {
    // edges: 0, the 1/2/3-byte boundaries, and the u64 max.
    uint64_t vals[] = { 0, 1, 127, 128, 16383, 16384,
                        0xFFFFFFFFull, 0xFFFFFFFFFFFFFFFFull };
    serialize_writer w; serialize_writer_init(&w);
    for (size_t i = 0; i < sizeof vals / sizeof *vals; i++)
        serialize_put_varint(&w, vals[i]);

    size_t len; uint8_t *buf = serialize_writer_take(&w, &len);
    serialize_reader r; serialize_reader_init(&r, buf, len);
    for (size_t i = 0; i < sizeof vals / sizeof *vals; i++) {
        uint64_t got = serialize_get_varint(&r);
        CHECK(got == vals[i], "varint %zu: got %llu want %llu",
              i, (unsigned long long)got, (unsigned long long)vals[i]);
    }
    CHECK(r.err == SERIALIZE_OK, "varint reader clean");
    free(buf);

    // a runaway varint (all continuation bits) must trip OVERFLOW, not spin.
    uint8_t bad[12]; memset(bad, 0x80, sizeof bad);
    serialize_reader br; serialize_reader_init(&br, bad, sizeof bad);
    serialize_get_varint(&br);
    CHECK(br.err == SERIALIZE_ERR_OVERFLOW, "runaway varint should overflow");
}

static void test_underrun(void) {
    uint8_t small[2] = { 1, 2 };
    serialize_reader r; serialize_reader_init(&r, small, sizeof small);
    serialize_get_u32(&r);                       // asks for 4, only 2 there
    CHECK(r.err == SERIALIZE_ERR_UNDERRUN, "short read should underrun");
    // sticky: a follow-up read stays zero and keeps the flag.
    CHECK(serialize_get_u8(&r) == 0, "reads after error return zero");
    CHECK(r.err == SERIALIZE_ERR_UNDERRUN, "error flag sticks");
}

static void test_blob_str(void) {
    serialize_writer w; serialize_writer_init(&w);
    const char *msg = "voxl save";
    serialize_put_str(&w, msg);
    serialize_put_str(&w, "");
    uint8_t payload[3] = { 9, 8, 7 };
    serialize_put_blob(&w, payload, sizeof payload);

    size_t len; uint8_t *buf = serialize_writer_take(&w, &len);
    serialize_reader r; serialize_reader_init(&r, buf, len);

    char tmp[32];
    serialize_get_str(&r, tmp, sizeof tmp);
    CHECK(strcmp(tmp, msg) == 0, "str roundtrip");
    serialize_get_str(&r, tmp, sizeof tmp);
    CHECK(tmp[0] == 0, "empty str roundtrip");

    size_t bl = 0;
    const uint8_t *bp = serialize_get_blob(&r, &bl);
    CHECK(bl == 3 && bp && bp[0] == 9 && bp[2] == 7, "blob roundtrip");
    free(buf);
}

static void test_tag_and_crc(void) {
    serialize_writer w; serialize_writer_init(&w);
    serialize_tag_scope s = serialize_tag_begin(&w, SERIALIZE_FOURCC('T','E','S','T'),
                                                3, SERIALIZE_TAG_F_CRC);
    serialize_put_u32(&w, 0xCAFEBABEu);
    serialize_put_str(&w, "payload");
    serialize_tag_end(&w, &s);

    size_t len; uint8_t *buf = serialize_writer_take(&w, &len);

    serialize_reader r; serialize_reader_init(&r, buf, len);
    serialize_tag_hdr h;
    CHECK(serialize_tag_next(&r, &h) == 0, "tag header read");
    CHECK(h.tag == SERIALIZE_FOURCC('T','E','S','T'), "fourcc roundtrip");
    CHECK(h.version == 3, "tag version roundtrip");
    CHECK(serialize_get_u32(&r) == 0xCAFEBABEu, "tag body roundtrip");
    char tmp[16]; serialize_get_str(&r, tmp, sizeof tmp);
    CHECK(strcmp(tmp, "payload") == 0, "tag body str");
    CHECK(serialize_tag_skip_body(&r, &h) == 0, "crc validates");

    // flip a byte in the body and confirm the crc check fires.
    buf[12] ^= 0xFF;
    serialize_reader r2; serialize_reader_init(&r2, buf, len);
    serialize_tag_next(&r2, &h);
    serialize_skip(&r2, h.body_len);
    CHECK(serialize_tag_skip_body(&r2, &h) != 0, "corrupt crc should fail");
    free(buf);

    // crc32 of "123456789" is the famous 0xCBF43926.
    CHECK(serialize_crc32("123456789", 9) == 0xCBF43926u, "crc32 check value");
}

static void test_vec_quant(void) {
    serialize_writer w; serialize_writer_init(&w);
    vec3 v = { 1.0f, -2.5f, 0.0f };
    serialize_put_vec3(&w, v);
    serialize_put_vec3_q16(&w, v, 8.0f);

    size_t len; uint8_t *buf = serialize_writer_take(&w, &len);
    serialize_reader r; serialize_reader_init(&r, buf, len);

    vec3 a = serialize_get_vec3(&r);
    CHECK(a.x == 1.0f && a.y == -2.5f && a.z == 0.0f, "vec3 exact roundtrip");

    vec3 b = serialize_get_vec3_q16(&r, 8.0f);
    // quantized, so allow one step of error (8/32767).
    float eps = 8.0f / 32767.0f * 2.0f;
    CHECK(fabsf(b.x - v.x) < eps && fabsf(b.y - v.y) < eps, "vec3 q16 roundtrip");
    free(buf);
}

int serialize_run_tests(void) {
    g_fail = 0;
    test_primitives();
    test_varint();
    test_underrun();
    test_blob_str();
    test_tag_and_crc();
    test_vec_quant();
    if (g_fail) LOGW("serialize_test: %d failure(s)", g_fail);
    else        LOGI("serialize_test: all green");
    return g_fail;
}
