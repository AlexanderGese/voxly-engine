#ifndef NET_COMPRESS_CONTAINER_H
#define NET_COMPRESS_CONTAINER_H
#include <stddef.h>
#include <stdint.h>
#include "compress.h"
#define COMPRESS_MAX_STAGES 4
typedef struct {
    uint16_t magic;
    uint8_t  version;
    uint8_t  method_count;
    uint8_t  methods[COMPRESS_MAX_STAGES];
    uint32_t raw_count;
    uint32_t payload_len;
    uint32_t crc32;
} compress_container;
#endif
