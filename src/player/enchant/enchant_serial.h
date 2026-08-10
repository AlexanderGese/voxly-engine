#ifndef PLAYER_ENCHANT_SERIAL_H
#define PLAYER_ENCHANT_SERIAL_H

#include "enchant_types.h"
#include <stddef.h>
#include <stdint.h>

// flat little-endian serialization for an enchant_set, so items with enchants
// survive a save/load round trip. deliberately standalone (a tiny cursor over
// a byte buffer) rather than leaning on util/serialize, because this lives
// inside item blobs and i didn't want the tag-wrapper overhead per item.
//
// wire format:
// u8  version
// u8  count
// repeat count times: u8 id, u8 level
//
// so a no-enchant item is two bytes. cheap enough to always write.

#define ENCHANT_SERIAL_VERSION 1

// bytes needed to write `s`. always 2 + 2*count.
size_t enchant_serial_size(const enchant_set *s);

// write `s` into buf (must hold enchant_serial_size bytes). returns the number
// of bytes written, or 0 if buf is too small.
size_t enchant_serial_write(const enchant_set *s, uint8_t *buf, size_t cap);

// read a set from buf. returns bytes consumed, or 0 on a malformed blob. out
// is cleared first. unknown ids are skipped rather than rejected so a newer
// save opened by an older build degrades gracefully.
size_t enchant_serial_read(enchant_set *out, const uint8_t *buf, size_t len);

#endif
