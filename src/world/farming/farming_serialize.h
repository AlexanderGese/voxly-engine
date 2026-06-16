#ifndef WORLD_FARMING_SERIALIZE_H
#define WORLD_FARMING_SERIALIZE_H

#include "farming_field.h"
#include <stddef.h>
#include <stdint.h>

// persistence for the field manager. crops and farmland tiles arent stored in
// the chunk block arrays (theyre logical records keyed by coord), so they need
// their own little blob in the save next to everything else. this is a flat,
// versioned, little-endian dump. no crc here; the doc layer that wraps the
// whole save already crc's the section.
//
// layout:
// u32 magic 'FARM'
// u16 version
// u16 reserved
// u32 seed
// u32 tick
// u32 tile_count
// u32 crop_count
// tile[tile_count]   (see .c for field order)
// crop[crop_count]
//
// the two record streams are independent; a crop refers to its tile only by
// shared coords, which the loader re-links by re-deriving the key.

#define FARMING_SAVE_MAGIC    0x4D524146u /* 'FARM' */
#define FARMING_SAVE_VERSION  1

// serialise the whole field into a freshly malloc'd buffer. *out_len gets the
// byte count. caller frees the buffer. returns NULL on allocation failure.
uint8_t *farming_serialize_write(const farming_field *f, size_t *out_len);

// load a field from a blob produced by the writer. clears any existing records
// in `f` first (it must already be farming_field_init'd). returns 0 on success,
// negative on a malformed/short/wrong-version blob.
int farming_serialize_read(farming_field *f, const uint8_t *data, size_t len);

// byte size a write would produce, without writing. handy for the save planner
// that pre-sizes its section table.
size_t farming_serialize_size(const farming_field *f);

#endif
