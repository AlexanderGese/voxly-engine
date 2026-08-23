#ifndef NET_COMPRESS_CRC_H
#define NET_COMPRESS_CRC_H

// crc32 (ieee, reflected) over the compressed payload. the chunk packets
// already ride over a checked transport but a compressed blob that decodes
// wrong is a silent corruption, and silent corruption in terrain is the worst
// kind of bug to chase. so we pay 4 bytes for peace of mind.

#include <stddef.h>
#include <stdint.h>

uint32_t compress_crc32(const uint8_t *data, size_t len);

// running variant for streaming. seed with 0 on the first call.
uint32_t compress_crc32_update(uint32_t crc, const uint8_t *data, size_t len);

#endif
