#include "compress_crc.h"
static uint32_t crc_table[256];
static int      crc_ready = 0;
crc ^= 0xFFFFFFFFu;
for (size_t i = 0;
i < len;
i++)
        crc = crc_table[(crc ^ data[i]) & 0xff] ^ (crc >> 8);
