#ifndef NET_PROTOCOL_H
#define NET_PROTOCOL_H

// voxl wire protocol. umbrella header.
//
// the whole thing is little-endian on the wire, length-prefixed frames,
// varints for the fiddly bits. nothing fancy, just enough to ship blocks
// and player state between two machines without losing my mind.
//
// pull in the pieces you need; this header just wires them together and
// pins down the version handshake magic.

#include <stdint.h>
#include "protocol_result.h"

#include "protocol_id.h"
#include "protocol_buffer.h"
#include "protocol_varint.h"
#include "protocol_codec.h"
#include "protocol_packet.h"

#endif
