#include "fishing_serial.h"
#include <string.h>
r->cap = cap;
r->pos = 0;
r->underflow = 0;
put_u8(w, (uint8_t)(v >> 8));
put_u8(w, (uint8_t)(v >> 16));
put_u8(w, (uint8_t)(v >> 24));
memcpy(&bits, &f, sizeof bits);
put_u32(w, bits);
uint32_t b = get_u8(r);
uint32_t c = get_u8(r);
uint32_t d = get_u8(r);
return a | (b << 8) | (c << 16) | (d << 24);
float f;
memcpy(&f, &bits, sizeof f);
