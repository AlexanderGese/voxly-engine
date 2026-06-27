#include "stronghold_query.h"
#include "stronghold_box.h"

int stronghold_query_room_at(const stronghold_graph *g, int x, int y, int z) {
    for (int i = 0; i < g->room_count; i++)
        if (stronghold_box_contains(&g->rooms[i].box, x, y, z)) return i;
    return -1;
}

long stronghold_query_room_dist_sq(const stronghold_graph *g, int room, int x, int y, int z) {
    int cx, cy, cz;
    stronghold_box_center(&g->rooms[room].box, &cx, &cy, &cz);
    long dx = cx - x, dy = cy - y, dz = cz - z;
    return dx * dx + dy * dy + dz * dz;
}

int stronghold_query_nearest_room(const stronghold_graph *g, int x, int y, int z) {
    if (g->room_count == 0) return -1;
    int best = 0;
    long best_d = stronghold_query_room_dist_sq(g, 0, x, y, z);
    for (int i = 1; i < g->room_count; i++) {
        long d = stronghold_query_room_dist_sq(g, i, x, y, z);
        if (d < best_d) { best_d = d; best = i; }
    }
    return best;
}

// shared bfs. fills parent[] (predecessor on the shortest path) and dist[].
// returns 1 if `to` was reached. caller-sized arrays of STRONGHOLD_MAX_ROOMS.
static int bfs(const stronghold_graph *g, int from, int to, int *parent, int *dist) {
    for (int i = 0; i < g->room_count; i++) { parent[i] = -1; dist[i] = -1; }
    if (from < 0 || from >= g->room_count) return 0;

    int queue[STRONGHOLD_MAX_ROOMS];
    int head = 0, tail = 0;
    dist[from] = 0;
    queue[tail++] = from;

    while (head < tail) {
        int cur = queue[head++];
        if (cur == to) return 1;   // early out, parents already set
        for (int e = 0; e < g->edge_count; e++) {
            int nb = -1;
            if (g->edges[e].a == cur) nb = g->edges[e].b;
            else if (g->edges[e].b == cur) nb = g->edges[e].a;
            if (nb < 0 || dist[nb] >= 0) continue;
            dist[nb] = dist[cur] + 1;
            parent[nb] = cur;
            queue[tail++] = nb;
        }
    }
    return to >= 0 && to < g->room_count && dist[to] >= 0;
}

int stronghold_query_hops(const stronghold_graph *g, int from, int to) {
    if (from < 0 || to < 0 || from >= g->room_count || to >= g->room_count) return -1;
    if (from == to) return 0;
    int parent[STRONGHOLD_MAX_ROOMS], dist[STRONGHOLD_MAX_ROOMS];
    bfs(g, from, to, parent, dist);
    return dist[to];   // -1 if unreachable
}

int stronghold_query_path(const stronghold_graph *g, int from, int to,
                          int *out_path, int cap) {
    if (!out_path || cap <= 0) return 0;
    if (from < 0 || to < 0 || from >= g->room_count || to >= g->room_count) return 0;

    int parent[STRONGHOLD_MAX_ROOMS], dist[STRONGHOLD_MAX_ROOMS];
    bfs(g, from, to, parent, dist);
    if (dist[to] < 0) return 0;   // unreachable

    int len = dist[to] + 1;
    if (len > cap) return 0;      // wont fit, dont half-fill it

    // walk parents back from `to`, then reverse into out_path.
    int cur = to;
    for (int i = len - 1; i >= 0; i--) {
        out_path[i] = cur;
        cur = parent[cur];
    }
    return len;
}

long stronghold_query_air_volume(const stronghold_graph *g) {
    long vol = 0;
    for (int i = 0; i < g->room_count; i++) {
        // interior is the footprint inset by one (the brick shell).
        stronghold_box in = stronghold_box_inset(g->rooms[i].box, -1, -1);
        int w = stronghold_box_width(&in);
        int h = stronghold_box_height(&in);
        int d = stronghold_box_depth(&in);
        if (w > 0 && h > 0 && d > 0) vol += (long)w * h * d;
    }
    return vol;
}

int stronghold_query_portal_depth(const stronghold_graph *g) {
    if (g->portal_room < 0) return -1;
    return stronghold_query_hops(g, 0, g->portal_room);
}
