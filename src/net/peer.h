#ifndef NET_PEER_H
#define NET_PEER_H

#include <stdint.h>

// peer tracking for the abandoned multiplayer attempt.
// each peer has a socket, a name, and a state machine.

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

typedef struct {
    net_peer peers[NET_MAX_PEERS];
    int      count;
    int      max_peers;
} peer_list;

void peer_list_init(peer_list *pl, int max);
int  peer_add(peer_list *pl, int fd, const char *name);
void peer_remove(peer_list *pl, int idx);
net_peer *peer_by_id(peer_list *pl, uint32_t id);
net_peer *peer_by_fd(peer_list *pl, int fd);
void peer_update_pos(net_peer *p, float x, float y, float z, float yaw, float pitch);
void peer_tick(peer_list *pl, float dt);

#endif
