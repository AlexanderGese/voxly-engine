#ifndef UTIL_SERIALIZE_CHUNK_H
#define UTIL_SERIALIZE_CHUNK_H

// chunk <-> bytes, on top of the tag layer. a chunk section is a 'CHNK' tag
// whose body is:
//
// i32 cx, i32 cz
// u8  block_enc    (SERIALIZE_BLK_*)
// u8  has_light
// ... block stream (rle or palette, see .c)
// ... light stream (rle, only if has_light)
//
// this is deliberately the same shape region_codec uses so a region file and
// a standalone save speak the same dialect; the difference is just the tag
// wrapper and crc that the doc layer adds around it.

#include "serialize_writer.h"
#include "serialize_reader.h"
#include "../../world/chunk.h"

enum {
    SERIALIZE_BLK_RAW = 0,   // CHUNK_VOLUME raw bytes, the fallback
    SERIALIZE_BLK_RLE = 1,   // run length
    SERIALIZE_BLK_PAL = 2    // palette + bitpacked indices
};

#define SERIALIZE_CHUNK_VERSION 1

// writes a full 'CHNK' section for c. picks whichever block encoding is
// smaller. include_light copies the light array too.
void serialize_chunk_write(serialize_writer *w, const chunk *c, int include_light);

// reads a 'CHNK' body into c (cx/cz are validated against c if c->cx/cz are
// set; pass a fresh chunk to accept whatever coords are stored). the reader
// must already be positioned at the body start (after serialize_tag_next).
// returns 0 on success.
int serialize_chunk_read(serialize_reader *r, chunk *c);

#endif
