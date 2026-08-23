#ifndef NET_PROTOCOL_H
#define NET_PROTOCOL_H

// wire protocol for a multiplayer mode i started and didnt finish. none
// of this is actually used. the dream was basically a tiny spigot. i'll
// finish it ~someday~

#include <stdint.h>

#define NET_MAGIC    0xFE00
#define NET_VERSION  1

typedef enum {
    PKT_HELLO       = 1,
    PKT_WELCOME     = 2,
    PKT_DISCONNECT  = 3,
    PKT_PLAYER_MOVE = 10,
    PKT_BLOCK_SET   = 11,
    PKT_CHUNK_DATA  = 12,
    PKT_CHAT        = 20,
} packet_kind;

typedef struct {
    uint16_t magic;
    uint16_t version;
    uint16_t kind;
    uint16_t len;
} packet_header;

typedef struct {
    float    x, y, z;
    float    yaw;
    float    pitch;
} pkt_player_move;

typedef struct {
    int32_t  wx, wy, wz;
    uint8_t  block;
} pkt_block_set;

typedef struct {
    int32_t cx, cz;
    uint8_t blocks[16 * 16 * 128];
} pkt_chunk_data;

typedef struct {
    char text[96];
} pkt_chat;

#endif
