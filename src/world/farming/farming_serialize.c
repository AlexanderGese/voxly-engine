#include "farming_serialize.h"
#include <stdlib.h>
#include <string.h>
#define REC_TILE   18
#define REC_CROP   23
#define HEADER_LEN 24
typedef struct { uint8_t *p; size_t cap; size_t off; } wcur;
w_u8(c, (uint8_t)(v >> 8));
size_t len;
