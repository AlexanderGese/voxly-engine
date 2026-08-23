#include "peer.h"

#include <string.h>

void peer_list_init(peer_list *pl, int max) {
    memset(pl, 0, sizeof *pl);
    pl->max_peers = max < NET_MAX_PEERS ? max : NET_MAX_PEERS;
}

int peer_add(peer_list *pl, int fd, const char *name) {
    if (pl->count >= pl->max_peers) return -1;
    for (int i = 0; i < NET_MAX_PEERS; i++) {
        if (pl->peers[i].state == PEER_DISCONNECTED) {
            net_peer *p = &pl->peers[i];
            memset(p, 0, sizeof *p);
            p->fd = fd;
            p->state = PEER_CONNECTING;
            strncpy(p->name, name, NET_NAME_MAX - 1);
            p->player_id = (uint32_t)(i + 1);
            pl->count++;
            return i;
        }
    }
    return -1;
}

void peer_remove(peer_list *pl, int idx) {
    if (idx < 0 || idx >= NET_MAX_PEERS) return;
    if (pl->peers[idx].state == PEER_DISCONNECTED) return;
    pl->peers[idx].state = PEER_DISCONNECTED;
    pl->peers[idx].fd = -1;
    pl->count--;
}

net_peer *peer_by_id(peer_list *pl, uint32_t id) {
    for (int i = 0; i < NET_MAX_PEERS; i++) {
        if (pl->peers[i].player_id == id && pl->peers[i].state != PEER_DISCONNECTED)
            return &pl->peers[i];
    }
    return NULL;
}

net_peer *peer_by_fd(peer_list *pl, int fd) {
    for (int i = 0; i < NET_MAX_PEERS; i++) {
        if (pl->peers[i].fd == fd && pl->peers[i].state != PEER_DISCONNECTED)
            return &pl->peers[i];
    }
    return NULL;
}

void peer_update_pos(net_peer *p, float x, float y, float z, float yaw, float pitch) {
    p->pos_x = x;
    p->pos_y = y;
    p->pos_z = z;
    p->yaw   = yaw;
    p->pitch = pitch;
    p->last_seen = 0;
}

void peer_tick(peer_list *pl, float dt) {
    for (int i = 0; i < NET_MAX_PEERS; i++) {
        net_peer *p = &pl->peers[i];
        if (p->state == PEER_DISCONNECTED) continue;
        p->last_seen += dt;
        if (p->last_seen > 30.0f) {
            p->state = PEER_KICKED;
        }
    }
}
