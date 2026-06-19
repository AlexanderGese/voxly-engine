#ifndef WORLD_LIGHTPROP_CODEC_H
#define WORLD_LIGHTPROP_CODEC_H

#include "lightprop_types.h"
#include <stddef.h>

// (de)serialize a chunk's packed light array. light is the single most
// run-heavy array in a chunk -- big slabs of "15 sky / 0 block" up top and "0/0"
// down in the dark -- so plain RLE on the byte-packed nibbles wins big and saves
// us re-flooding every chunk on load. this is its own little codec so the save
// layer can opt in without lightprop pulling in save.h.
//
// wire format (per chunk): sequence of [count:u16][value:u8] runs covering all
// CHUNK_VOLUME bytes of c->light. count is 1-based (a stored 0 means 1) so a run
// can be up to 65536 long. simple, robust, no dictionary.

// worst case the encoded form is 1.5x the input (every byte its own run). give
// the caller a bound so it can size a buffer without guessing.
size_t lp_codec_bound(void);

// encode c->light into dst (must hold >= lp_codec_bound()). returns bytes written.
size_t lp_codec_encode(const chunk *c, uint8_t *dst, size_t dst_cap);

// decode `len` bytes from src back into c->light. returns 1 on success, 0 if the
// stream is malformed or doesn't cover exactly CHUNK_VOLUME bytes.
int    lp_codec_decode(chunk *c, const uint8_t *src, size_t len);

// quick integrity probe: does encoding then decoding round-trip this chunk? used
// in tests / asserts, not the hot path.
int    lp_codec_roundtrip_ok(const chunk *c);

#endif
