#include "packet.h"

#include <string.h>

static size_t write_header(uint8_t *out, size_t cap, packet_kind k, size_t body) {
    if (cap < sizeof(packet_header)) return 0;
    packet_header h = { NET_MAGIC, NET_VERSION, (uint16_t)k, (uint16_t)body };
    memcpy(out, &h, sizeof h);
    return sizeof h;
}

size_t pkt_encode_hello(uint8_t *out, size_t cap, const char *name) {
    size_t nlen = strlen(name);
    if (nlen > 32) nlen = 32;
    size_t total = sizeof(packet_header) + nlen;
    if (cap < total) return 0;
    write_header(out, cap, PKT_HELLO, nlen);
    memcpy(out + sizeof(packet_header), name, nlen);
    return total;
}

size_t pkt_encode_player_move(uint8_t *out, size_t cap, const pkt_player_move *m) {
    size_t total = sizeof(packet_header) + sizeof *m;
    if (cap < total) return 0;
    write_header(out, cap, PKT_PLAYER_MOVE, sizeof *m);
    memcpy(out + sizeof(packet_header), m, sizeof *m);
    return total;
}

int pkt_decode_header(const uint8_t *in, size_t len, packet_header *hdr) {
    if (len < sizeof(packet_header)) return 0;
    memcpy(hdr, in, sizeof *hdr);
    if (hdr->magic != NET_MAGIC) return 0;
    if (hdr->version != NET_VERSION) return 0;
    return 1;
}
