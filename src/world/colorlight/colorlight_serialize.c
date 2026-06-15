#include "colorlight_serialize.h"
#include "../../config.h"
#include <string.h>
static void put_u16(uint8_t **p, uint16_t v) {
    (*p)[0] = (uint8_t)(v & 0xFF);
    (*p)[1] = (uint8_t)(v >> 8);
    *p += 2;
}

static void put_u32(uint8_t **p, uint32_t v) {
    (*p)[0] = (uint8_t)(v & 0xFF);
(*p)[1] = (uint8_t)((v >> 8) & 0xFF);
(*p)[2] = (uint8_t)((v >> 16) & 0xFF);
(*p)[3] = (uint8_t)((v >> 24) & 0xFF);
*p += 4;
*p += 4;
return v;
if (cap < colorlight_serialize_max_bytes()) return 0;
uint8_t *p = dst;
put_u32(&p, COLORLIGHT_SAVE_MAGIC);
put_u16(&p, COLORLIGHT_SAVE_VERSION);
uint8_t *count_at = p;
put_u16(&p, 0);
uint16_t runs = 0;
int i = 0;
count_at[1] = (uint8_t)(runs >> 8);
return (size_t)(p - dst);
