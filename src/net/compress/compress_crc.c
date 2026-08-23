#include "compress_crc.h"

// lazily built table. standard reflected poly 0xEDB88320. building it once at
// first use saves shipping a 1k literal array and reads about the same.
static uint32_t crc_table[256];
static int      crc_ready = 0;

static void crc_build(void) {
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int k = 0; k < 8; k++)
            c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
        crc_table[i] = c;
    }
    crc_ready = 1;
}

uint32_t compress_crc32_update(uint32_t crc, const uint8_t *data, size_t len) {
    if (!crc_ready) crc_build();
    crc ^= 0xFFFFFFFFu;
    for (size_t i = 0; i < len; i++)
        crc = crc_table[(crc ^ data[i]) & 0xff] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFFu;
}

uint32_t compress_crc32(const uint8_t *data, size_t len) {
    return compress_crc32_update(0, data, len);
}
