#include "compress_container.h"
#include "compress_stream.h"

size_t compress_container_write_header(uint8_t *out, size_t cap,
                                       const compress_container *c) {
    compress_wstream ws;
    compress_ws_init(&ws, out, cap);

    compress_ws_u16(&ws, c->magic);
    compress_ws_u8(&ws, c->version);
    compress_ws_u8(&ws, c->method_count);
    for (uint8_t i = 0; i < c->method_count; i++)
        compress_ws_u8(&ws, c->methods[i]);
    compress_ws_var(&ws, c->raw_count);
    compress_ws_var(&ws, c->payload_len);
    compress_ws_u32(&ws, c->crc32);

    if (ws.err) return 0;
    return ws.pos;
}

size_t compress_container_read_header(const uint8_t *in, size_t len,
                                      compress_container *c) {
    compress_rstream rs;
    compress_rs_init(&rs, in, len);

    c->magic   = compress_rs_u16(&rs);
    c->version = compress_rs_u8(&rs);
    if (c->magic != COMPRESS_MAGIC || c->version != COMPRESS_VERSION)
        return 0;

    c->method_count = compress_rs_u8(&rs);
    if (c->method_count > COMPRESS_MAX_STAGES) return 0;
    for (uint8_t i = 0; i < c->method_count; i++) {
        c->methods[i] = compress_rs_u8(&rs);
        if (c->methods[i] >= COMPRESS_M_COUNT) return 0;
    }

    c->raw_count   = compress_rs_var(&rs);
    c->payload_len = compress_rs_var(&rs);
    c->crc32       = compress_rs_u32(&rs);

    if (rs.err) return 0;
    return rs.pos;
}
