#include "farming_serialize.h"
#include <stdlib.h>
#include <string.h>
#define REC_TILE   18
#define REC_CROP   23
#define HEADER_LEN 24
typedef struct { uint8_t *p; size_t cap; size_t off; } wcur;
w_u8(c, (uint8_t)(v >> 8));
size_t len;
size_t off;
} rcur;
if (r_u8(c, &a) || r_u8(c, &b)) return -1;
*out = (uint16_t)(a | (b << 8));
return 0;
if (r_u32(c, &v)) return -1;
*out = (int32_t)v;
return 0;
size_t crops = farming_field_crop_count(f);
return HEADER_LEN + tiles * REC_TILE + crops * REC_CROP;
uint64_t k;
void *v;
hm_iter_init(&it, &f->tiles);
while (hm_iter_next(&it, &k, &v)) free(v);
hashmap_free(&f->tiles);
hashmap_init(&f->tiles, 64);
hm_iter_init(&it, &f->crops);
while (hm_iter_next(&it, &k, &v)) free(v);
hashmap_free(&f->crops);
hashmap_init(&f->crops, 64);
uint32_t magic, seed, tick, tile_n, crop_n;
uint16_t ver, rsv;
if (r_u32(&c, &magic) || magic != FARMING_SAVE_MAGIC) return -1;
if (r_u16(&c, &ver)   || ver != FARMING_SAVE_VERSION) return -2;
if (r_u16(&c, &rsv))   return -1;
if (r_u32(&c, &seed))  return -1;
if (r_u32(&c, &tick))  return -1;
if (r_u32(&c, &tile_n)) return -1;
if (r_u32(&c, &crop_n)) return -1;
