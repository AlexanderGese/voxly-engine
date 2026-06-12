#ifndef UTIL_SERIALIZE_TAG_H
#define UTIL_SERIALIZE_TAG_H

// tagged blobs (tlv-ish). the spine of every save file.
//
// a tag is a 4-char fourcc ('CHNK', 'ENTS', ...). on the wire a section is:
//
// u32  tag        (fourcc, the 4 chars verbatim in order)
// u8   version    (per-section, NOT the abi)
// u8   flags      (SERIALIZE_TAG_F_*)
// u32  body_len   (bytes of body that follow, after this field)
// ...  body
// [u32 crc32]     present iff F_CRC, covers tag..body
//
// readers walk sections in order. an unknown tag is skipped using body_len so
// old saves load in new builds and new saves degrade gracefully in old ones.
// thats the whole point of carrying the length.

#include "serialize_writer.h"
#include "serialize_reader.h"

#define SERIALIZE_FOURCC(a,b,c,d) \
    ((uint32_t)(a) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))

enum {
    SERIALIZE_TAG_F_NONE = 0,
    SERIALIZE_TAG_F_CRC  = 1 << 0,   // trailing crc32 of the section
    SERIALIZE_TAG_F_ZPAD = 1 << 1    // body was zero-trimmed (reader memsets)
};

typedef struct {
    uint32_t tag;
    uint8_t  version;
    uint8_t  flags;
    size_t   len_at;     // writer offset of body_len, for patching
    size_t   body_at;    // writer offset where body starts
} serialize_tag_scope;

// --- writer side ---------------------------------------------------------
// begin/end bracket a section. write the body in between with normal puts.
serialize_tag_scope serialize_tag_begin(serialize_writer *w, uint32_t tag,
                                        uint8_t version, uint8_t flags);
void serialize_tag_end(serialize_writer *w, serialize_tag_scope *scope);

// --- reader side ---------------------------------------------------------
typedef struct {
    uint32_t tag;
    uint8_t  version;
    uint8_t  flags;
    size_t   body_len;
    size_t   body_at;    // reader tell() at body start
} serialize_tag_hdr;

// peek/read the next section header. returns 0 on success. on success the
// reader cursor sits at the start of the body. body_len tells you how far.
int serialize_tag_next(serialize_reader *r, serialize_tag_hdr *out);

// advance the reader to just past a section whose header you already read,
// regardless of how much of the body you consumed. also verifies crc if set.
int serialize_tag_skip_body(serialize_reader *r, const serialize_tag_hdr *h);

// convenience: scan forward until a section with `tag` is found, leaving the
// cursor at its body. returns 0 on hit, -1 if not found before eof.
int serialize_tag_find(serialize_reader *r, uint32_t tag, serialize_tag_hdr *out);

#endif
