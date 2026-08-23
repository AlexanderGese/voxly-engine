#ifndef NET_PEER_H
#define NET_PEER_H
#include <stdint.h>
#define NET_MAX_PEERS 16
#define NET_NAME_MAX  32
typedef enum {
    PEER_DISCONNECTED = 0,
    PEER_CONNECTING,
    PEER_HANDSHAKE,
    PEER_CONNECTED,
    PEER_KICKED,
} peer_state;
typedef struct {
    int        fd;
    peer_state state;
    char       name[NET_NAME_MAX];
    uint32_t   player_id;
    float      last_seen;
    float      ping;
    float      pos_x, pos_y, pos_z;
    float      yaw, pitch;
} net_peer;
#endif
