#ifndef WORLD_REGION_CODEC_H
#define WORLD_REGION_CODEC_H
#include "region_types.h"
#include "../chunk.h"
#include <stdint.h>
// chunk <-> byte buffer. this is the actual block/light packing, independent
// of where the bytes end up on disk. each encoded chunk starts with a small
// payload header:
//
// u32 total_len     (bytes following this field, for skip/validation)
// i32 cx
// i32 cz
// u8  enc           (REGION_ENC_*, also mirrored in the loc table)
// u8  has_light     (1 if a light section follows the block section)
// ... block section
// ... light section (optional)
typedef struct {
    uint8_t *data;      // malloc'd buffer
    size_t   len;       // bytes used
    size_t   cap;
    uint8_t  enc;       // chosen encoding for the block section
} region_blob_t;
void region_blob_init(region_blob_t *b);
void region_blob_free(region_blob_t *b);
int  region_codec_encode(region_blob_t *b, const chunk *c, int include_light);
int  region_codec_decode(chunk *c, const uint8_t *data, size_t len);
#endif
