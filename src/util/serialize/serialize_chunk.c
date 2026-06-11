#include "serialize_chunk.h"
#include "serialize_tag.h"
#include "../log.h"

#include <stdlib.h>
#include <string.h>

// --- block encoders ------------------------------------------------------

// rle: [varint run_count] then run_count * (u8 id, varint len)
static void enc_rle(serialize_writer *w, const block_id *blocks) {
    // count runs first so we can prefix the count (avoids a patch dance).
    size_t runs = 0;
    for (size_t i = 0; i < CHUNK_VOLUME; ) {
        block_id id = blocks[i];
        size_t len = 1;
        while (i + len < CHUNK_VOLUME && blocks[i + len] == id) len++;
        runs++;
        i += len;
    }
    serialize_put_varint(w, runs);
    for (size_t i = 0; i < CHUNK_VOLUME; ) {
        block_id id = blocks[i];
        size_t len = 1;
        while (i + len < CHUNK_VOLUME && blocks[i + len] == id) len++;
        serialize_put_u8(w, id);
        serialize_put_varint(w, len);
        i += len;
    }
}

// palette: [u8 pal_n][pal ids...][u8 bits] then lsb-first bitpacked indices.
// returns -1 if the chunk is too varied to be worth a palette.
static int enc_pal(serialize_writer *w, const block_id *blocks) {
    uint8_t pal[256];
    int     pal_n = 0;
    int     map[256];
    for (int i = 0; i < 256; i++) map[i] = -1;

    for (size_t i = 0; i < CHUNK_VOLUME; i++) {
        block_id id = blocks[i];
        if (map[id] < 0) {
            if (pal_n >= 64) return -1;
            map[id] = pal_n;
            pal[pal_n++] = id;
        }
    }

    int bits = 1;
    while ((1 << bits) < pal_n) bits++;

    serialize_put_u8(w, (uint8_t)pal_n);
    for (int i = 0; i < pal_n; i++) serialize_put_u8(w, pal[i]);
    serialize_put_u8(w, (uint8_t)bits);

    uint32_t acc = 0;
    int      acc_bits = 0;
    for (size_t i = 0; i < CHUNK_VOLUME; i++) {
        acc |= (uint32_t)map[blocks[i]] << acc_bits;
        acc_bits += bits;
        while (acc_bits >= 8) {
            serialize_put_u8(w, (uint8_t)(acc & 0xFF));
            acc >>= 8;
            acc_bits -= 8;
        }
    }
    if (acc_bits > 0) serialize_put_u8(w, (uint8_t)(acc & 0xFF));
    return 0;
}

// light always rle's well (huge flat runs of sky/dark), so dont bother
// measuring, just rle it.
static void enc_light_rle(serialize_writer *w, const uint8_t *light) {
    size_t runs = 0;
    for (size_t i = 0; i < CHUNK_VOLUME; ) {
        uint8_t v = light[i];
        size_t len = 1;
        while (i + len < CHUNK_VOLUME && light[i + len] == v) len++;
        runs++;
        i += len;
    }
    serialize_put_varint(w, runs);
    for (size_t i = 0; i < CHUNK_VOLUME; ) {
        uint8_t v = light[i];
        size_t len = 1;
        while (i + len < CHUNK_VOLUME && light[i + len] == v) len++;
        serialize_put_u8(w, v);
        serialize_put_varint(w, len);
        i += len;
    }
}

// --- public write --------------------------------------------------------

void serialize_chunk_write(serialize_writer *w, const chunk *c, int include_light) {
    serialize_tag_scope s = serialize_tag_begin(w, SERIALIZE_FOURCC('C','H','N','K'),
                                                SERIALIZE_CHUNK_VERSION,
                                                SERIALIZE_TAG_F_CRC);
    serialize_put_i32(w, c->cx);
    serialize_put_i32(w, c->cz);

    // we want to know which of rle/pal is smaller without writing twice into
    // the real buffer, so measure each into a throwaway writer.
    serialize_writer pw; serialize_writer_init(&pw);
    int pal_ok = enc_pal(&pw, c->blocks) == 0;
    size_t pal_len = pw.len;

    serialize_writer rw; serialize_writer_init(&rw);
    enc_rle(&rw, c->blocks);
    size_t rle_len = rw.len;

    if (pal_ok && pal_len <= rle_len) {
        serialize_put_u8(w, SERIALIZE_BLK_PAL);
        serialize_put_u8(w, include_light ? 1 : 0);
        serialize_put_bytes(w, pw.data, pw.len);
    } else {
        serialize_put_u8(w, SERIALIZE_BLK_RLE);
        serialize_put_u8(w, include_light ? 1 : 0);
        serialize_put_bytes(w, rw.data, rw.len);
    }
    serialize_writer_free(&pw);
    serialize_writer_free(&rw);

    if (include_light) enc_light_rle(w, c->light);

    serialize_tag_end(w, &s);
}

// --- decoders ------------------------------------------------------------

static int dec_rle(serialize_reader *r, block_id *out) {
    uint64_t runs = serialize_get_varint(r);
    size_t i = 0;
    for (uint64_t k = 0; k < runs; k++) {
        uint8_t id = serialize_get_u8(r);
        uint64_t len = serialize_get_varint(r);
        for (uint64_t j = 0; j < len && i < CHUNK_VOLUME; j++) out[i++] = id;
    }
    if (r->err != SERIALIZE_OK || i != CHUNK_VOLUME) return -1;
    return 0;
}

static int dec_pal(serialize_reader *r, block_id *out) {
    uint8_t pal_n = serialize_get_u8(r);
    if (pal_n == 0 || pal_n > 64) { r->err = SERIALIZE_ERR_CORRUPT; return -1; }
    uint8_t pal[64];
    for (int i = 0; i < pal_n; i++) pal[i] = serialize_get_u8(r);
    uint8_t bits = serialize_get_u8(r);
    if (bits == 0 || bits > 6) { r->err = SERIALIZE_ERR_CORRUPT; return -1; }

    uint32_t acc = 0;
    int      acc_bits = 0;
    uint32_t mask = (1u << bits) - 1;
    for (size_t i = 0; i < CHUNK_VOLUME; i++) {
        while (acc_bits < bits) {
            acc |= (uint32_t)serialize_get_u8(r) << acc_bits;
            acc_bits += 8;
        }
        uint32_t idx = acc & mask;
        acc >>= bits;
        acc_bits -= bits;
        if (idx >= pal_n) { r->err = SERIALIZE_ERR_CORRUPT; return -1; }
        out[i] = pal[idx];
    }
    return r->err == SERIALIZE_OK ? 0 : -1;
}

static int dec_light(serialize_reader *r, uint8_t *out) {
    uint64_t runs = serialize_get_varint(r);
    size_t i = 0;
    for (uint64_t k = 0; k < runs; k++) {
        uint8_t v = serialize_get_u8(r);
        uint64_t len = serialize_get_varint(r);
        for (uint64_t j = 0; j < len && i < CHUNK_VOLUME; j++) out[i++] = v;
    }
    if (r->err != SERIALIZE_OK || i != CHUNK_VOLUME) return -1;
    return 0;
}

int serialize_chunk_read(serialize_reader *r, chunk *c) {
    int32_t cx = serialize_get_i32(r);
    int32_t cz = serialize_get_i32(r);
    uint8_t enc = serialize_get_u8(r);
    uint8_t has_light = serialize_get_u8(r);
    if (r->err != SERIALIZE_OK) return -1;

    // if the caller pinned coords, enforce them. a fresh chunk (0,0) just
    // adopts whats on disk.
    if ((c->cx || c->cz) && (cx != c->cx || cz != c->cz)) {
        LOGW("serialize: chunk coord mismatch (%d,%d) want (%d,%d)",
             cx, cz, c->cx, c->cz);
        return -1;
    }
    c->cx = cx;
    c->cz = cz;

    int rc;
    switch (enc) {
        case SERIALIZE_BLK_RLE: rc = dec_rle(r, c->blocks); break;
        case SERIALIZE_BLK_PAL: rc = dec_pal(r, c->blocks); break;
        case SERIALIZE_BLK_RAW:
            rc = serialize_get_bytes(r, c->blocks, CHUNK_VOLUME);
            break;
        default:
            LOGW("serialize: unknown block enc %u", enc);
            return -1;
    }
    if (rc != 0) return -1;

    if (has_light) {
        if (dec_light(r, c->light) != 0) {
            // light is cheap to regenerate, dont fail the load over it.
            LOGW("serialize: bad light, will relight");
            memset(c->light, 0, sizeof c->light);
        }
    } else {
        memset(c->light, 0, sizeof c->light);
    }
    c->generated = 1;
    c->dirty = 1;
    return 0;
}
