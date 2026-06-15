#ifndef WORLD_COLORLIGHT_SERIALIZE_H
#define WORLD_COLORLIGHT_SERIALIZE_H

#include <stdint.h>
#include <stddef.h>
#include "colorlight_grid.h"

// rgb light persistence. the scalar save format (save.h) only stores blocks and
// recomputes scalar light on load; we COULD do the same for rgb, but a full
// chunk relight is the expensive part and most chunks have zero emitters, so
// caching the grid is worth it. format is a sidecar blob, not baked into the
// chunk file, so old saves still load.
//
// the grid is overwhelmingly black (only lit cells near torches are nonzero) so
// we rle the packed words: a run is [u16 value][u16 count]. a dark chunk
// compresses to a handful of bytes. order matches chunk_idx.
//
// blob layout:
// u32 magic   ('CLGT')
// u16 version
// u16 run_count
// run_count * { u16 value, u16 count }   ; counts sum to CHUNK_VOLUME

#define COLORLIGHT_SAVE_MAGIC   0x54474C43u  /* 'CLGT' little-endian */
#define COLORLIGHT_SAVE_VERSION 1

// upper bound on the encoded size, for sizing a scratch buffer. worst case is
// fully alternating cells: CHUNK_VOLUME runs of 1, plus the header.
size_t colorlight_serialize_max_bytes(void);

// encode a grid into dst (must be >= max_bytes). returns bytes written, or 0 on
// a null grid / buffer.
size_t colorlight_serialize_encode(const colorlight_grid *g, uint8_t *dst, size_t cap);

// decode into a grid (cells get overwritten). returns bytes consumed, or 0 if
// the blob is malformed / wrong magic / wrong run total.
size_t colorlight_serialize_decode(colorlight_grid *g, const uint8_t *src, size_t len);

#endif
