#ifndef NET_PACKET_H
#define NET_PACKET_H

#include "protocol.h"
#include <stddef.h>

// serialization helpers.

size_t pkt_encode_hello(uint8_t *out, size_t cap, const char *name);
size_t pkt_encode_player_move(uint8_t *out, size_t cap, const pkt_player_move *m);
int    pkt_decode_header(const uint8_t *in, size_t len, packet_header *hdr);

#endif
