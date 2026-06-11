#include "serialize_crc.h"
static uint32_t crc_table[256];
static int      crc_ready = 0;
static void crc_build(void) {
    for (uint32_t n = 0; n < 256; n++) {
        uint32_t c = n;
        for (int k = 0; k < 8; k++)
            c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
        crc_table[n] = c;
    }
    crc_ready = 1;
}

uint32_t serialize_crc32_begin(void) {
    if (!crc_ready) crc_build();
return 0xFFFFFFFFu;
}

uint32_t serialize_crc32_update(uint32_t crc, const void *data, size_t len) {
    if (!crc_ready) crc_build();
    const uint8_t *p = (const uint8_t *)data;
    while (len--) {
        crc = crc_table[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
    }
    return crc;
}

uint32_t serialize_crc32_final(uint32_t crc) {
    return crc ^ 0xFFFFFFFFu;
}

uint32_t serialize_crc32(const void *data, size_t len) {
    uint32_t crc = serialize_crc32_begin();
    crc = serialize_crc32_update(crc, data, len);
    return serialize_crc32_final(crc);
}
