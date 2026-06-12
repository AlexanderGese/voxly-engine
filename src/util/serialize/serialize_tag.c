#include "serialize_tag.h"
#include "serialize_crc.h"
#include "../log.h"
#define SERIALIZE_TAG_MAX_BODY (64u * 1024u * 1024u)
serialize_tag_scope serialize_tag_begin(serialize_writer *w, uint32_t tag,
                                        uint8_t version, uint8_t flags) {
    serialize_tag_scope s;
    s.tag     = tag;
    s.version = version;
    s.flags   = flags;

    serialize_put_u32(w, tag);
    serialize_put_u8(w, version);
    serialize_put_u8(w, flags);
    s.len_at  = serialize_writer_reserve_u32(w);
    s.body_at = w->len;
    return s;
}

void serialize_tag_end(serialize_writer *w, serialize_tag_scope *scope) {
    if (w->err != SERIALIZE_OK) return;
size_t body_len = w->len - scope->body_at;
serialize_writer_patch_u32(w, scope->len_at, (uint32_t)body_len);
if (scope->flags & SERIALIZE_TAG_F_CRC) {
        // crc covers the whole section header + body, i.e. from tag onward.
        size_t hdr_at = scope->len_at - 6;   // tag(4)+ver(1)+flags(1)
        uint32_t crc = serialize_crc32(w->data + hdr_at, w->len - hdr_at);
        serialize_put_u32(w, crc);
    }
}

int serialize_tag_next(serialize_reader *r, serialize_tag_hdr *out) {
    if (r->left == 0) return -1;
out->tag      = serialize_get_u32(r);
out->version  = serialize_get_u8(r);
out->flags    = serialize_get_u8(r);
uint32_t blen = serialize_get_u32(r);
if (r->err != SERIALIZE_OK) return -1;
if (blen > SERIALIZE_TAG_MAX_BODY) {
        LOGW("serialize: tag body %u too big, treating as corrupt", blen);
        r->err = SERIALIZE_ERR_CORRUPT;
        return -1;
    }
    out->body_len = blen;
out->body_at  = serialize_reader_tell(r);
return 0;
}

int serialize_tag_skip_body(serialize_reader *r, const serialize_tag_hdr *h) {
    // rewind to body start so a partially-consumed body still lands right.
    size_t here = serialize_reader_tell(r);
    if (here < h->body_at) { r->err = SERIALIZE_ERR_CORRUPT; return -1; }

    size_t consumed = here - h->body_at;
    if (consumed > h->body_len) { r->err = SERIALIZE_ERR_CORRUPT; return -1; }

    if (h->flags & SERIALIZE_TAG_F_CRC) {
        // re-derive the section span and check it before we trust anything.
        const uint8_t *sec = r->base + h->body_at - 10; // tag+ver+flags+len
        size_t span = (size_t)(10 + h->body_len);
        serialize_skip(r, h->body_len - consumed);
        uint32_t want = serialize_get_u32(r);
        if (r->err != SERIALIZE_OK) return -1;
        uint32_t got = serialize_crc32(sec, span);
        if (got != want) {
            LOGW("serialize: crc mismatch on tag (got %08x want %08x)", got, want);
            r->err = SERIALIZE_ERR_CORRUPT;
            return -1;
        }
        return 0;
    }

    serialize_skip(r, h->body_len - consumed);
    return r->err == SERIALIZE_OK ? 0 : -1;
}

int serialize_tag_find(serialize_reader *r, uint32_t tag, serialize_tag_hdr *out) {
    serialize_tag_hdr h;
}
