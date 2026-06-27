#include "stronghold_graph.h"
#include <stddef.h>
// room sizing. rooms are squat boxes; halls are wider, the portal room is the
// biggest fixed-size box so the frame always fits.
#define ROOM_MIN_W   5
#define ROOM_MAX_W   9
#define ROOM_H       6
#define PORTAL_W    11
#define PORTAL_H     8
static int dir_bit(stronghold_dir d) { return 1 << (int)d; }

void stronghold_graph_init(stronghold_graph *g) {
    g->room_count = 0;
g->edge_count = 0;
g->portal_room = -1;
g->bounds = stronghold_box_make(0, 0, 0, 0, 0, 0);
}

// can this box go in without kissing an existing room? pad of 2 keeps a stone
// wall between adjacent rooms that arent meant to share one.
static int box_fits(const stronghold_graph *g, const stronghold_box *box) {
    for (int i = 0; i < g->room_count; i++) {
        if (stronghold_box_overlaps_pad(box, &g->rooms[i].box, 2)) return 0;
    }
    return 1;
}

// commit a room, return its index or -1 if the table is full.
static int push_room(stronghold_graph *g, stronghold_box box, int level, uint32_t seed) {
    if (g->room_count >= STRONGHOLD_MAX_ROOMS) return -1;
int idx = g->room_count++;
stronghold_room *r = &g->rooms[idx];
r->box = box;
r->type = STRONGHOLD_ROOM_HALL;
// typed later
r->level = level;
r->seed = seed;
r->open_sides = 0x0F;
// all four sides free
r->door_count = 0;
g->bounds = (g->room_count == 1) ? box : stronghold_box_union(g->bounds, box);
return idx;
}

static int push_edge(stronghold_graph *g, int a, int b, stronghold_dir side, int len, int stair) {
    if (g->edge_count >= STRONGHOLD_MAX_EDGES) return -1;
    int idx = g->edge_count++;
    stronghold_edge *e = &g->edges[idx];
    e->a = a; e->b = b; e->side_a = side; e->length = len; e->stair = stair;
    // both endpoints lose that side from their open mask.
    g->rooms[a].open_sides &= ~dir_bit(side);
    g->rooms[b].open_sides &= ~dir_bit(stronghold_dir_opposite(side));
    return idx;
}

void stronghold_room_add_door(stronghold_room *r, int x, int y, int z,
                              stronghold_dir facing, int gated) {
    if (r->door_count >= STRONGHOLD_ROOM_DOORS) return;
stronghold_door *d = &r->doors[r->door_count++];
d->x = x;
d->y = y;
d->z = z;
d->facing = facing;
d->gated = gated;
}

// pick a random still-open side of a room, or -1 if it's fully wired.
static int pick_open_side(const stronghold_room *r, stronghold_rng *rng) {
    int sides[4], n = 0;
    for (int d = 0; d < 4; d++)
        if (r->open_sides & dir_bit(d)) sides[n++] = d;
    if (n == 0) return -1;
    return sides[stronghold_rng_range(rng, 0, n - 1)];
}

// project a new room off `from` through `side`, separated by a corridor of
// length `len`. fills *out and *out_floor_y. caller checks fit.
static stronghold_box project_room(const stronghold_room *from, stronghold_dir side,
                                   int len, int w, int d, int drop, int *out_y) {
    int dx, dz;
stronghold_dir_step(side, &dx, &dz);
// start from from's center face, walk len+gap blocks outward, drop in y.
int fcx, fcy, fcz;
stronghold_box fb = from->box;
stronghold_box_center(&fb, &fcx, &fcy, &fcz);
int reach = (dx ? stronghold_box_width(&fb) : stronghold_box_depth(&fb)) / 2 + len;
int nx = fcx + dx * reach;
int nz = fcz + dz * reach;
int ny = from->box.y0 - drop;
if (out_y) *out_y = ny;
// center the new room on the axis we walked.
return stronghold_box_at(nx - w / 2, ny, nz - d / 2, w, ROOM_H, d);
}

int stronghold_graph_grow(stronghold_graph *g, const stronghold_config *cfg,
                          int ox, int oy, int oz, stronghold_rng *rng) {
    stronghold_graph_init(g);

    int cap = cfg->max_rooms;
    if (cap > STRONGHOLD_MAX_ROOMS) cap = STRONGHOLD_MAX_ROOMS;

    // seed the first room around the origin. this becomes the spawn hub.
    int w0 = stronghold_rng_range(rng, ROOM_MIN_W, ROOM_MAX_W);
    int d0 = stronghold_rng_range(rng, ROOM_MIN_W, ROOM_MAX_W);
    stronghold_box first = stronghold_box_at(ox - w0 / 2, oy, oz - d0 / 2, w0, ROOM_H, d0);
    push_room(g, first, 0, stronghold_seed_mix(rng->s ? (uint32_t)rng->s : 1u, 0xa1));

    // bounded guided random walk. each pass picks a frontier room with at least
    // one open side and tries to bud a new room off it. we cap attempts so a
    // cramped frontier cant spin forever.
    int attempts = 0;
    int attempt_cap = cap * 12;
    while (g->room_count < cap && attempts < attempt_cap) {
        attempts++;

        // choose a source room, biased toward recent rooms so the maze spreads
        // outward instead of clumping at the origin.
        int src;
        if (stronghold_rng_chance(rng, 0.65f))
            src = g->room_count - 1 - stronghold_rng_range(rng, 0, g->room_count - 1) / 2;
        else
            src = stronghold_rng_range(rng, 0, g->room_count - 1);
        if (src < 0) src = 0;

        int side = pick_open_side(&g->rooms[src], rng);
        if (side < 0) continue;

        int len = stronghold_rng_range(rng, cfg->min_depth ? 3 : 3, 7);
        // occasionally drop a level via a stairwell-style corridor.
        int stair = 0, drop = 0;
        if (g->rooms[src].level + 1 < cfg->max_levels &&
            stronghold_rng_chance(rng, 0.18f)) {
            stair = 1;
            drop = cfg->level_height;
        }

        int w = stronghold_rng_range(rng, ROOM_MIN_W, ROOM_MAX_W);
        int d = stronghold_rng_range(rng, ROOM_MIN_W, ROOM_MAX_W);
        int ny;
        stronghold_box nb = project_room(&g->rooms[src], (stronghold_dir)side,
                                         len, w, d, drop, &ny);
        (void)ny;

        if (!box_fits(g, &nb)) continue;

        uint32_t rseed = stronghold_seed_mix((uint32_t)(uintptr_t)src + 1u,
                                             stronghold_rng_next(rng));
        int dst = push_room(g, nb, g->rooms[src].level + stair, rseed);
        if (dst < 0) break;
        push_edge(g, src, dst, (stronghold_dir)side, len, stair);
    }

    return g->room_count;
}

// depth-first distance from room 0 over the edge list. used to find the far
// dead-end that gets the portal. iterative, small fixed stack.
static int graph_depth_of(const stronghold_graph *g, int target) {
    int dist[STRONGHOLD_MAX_ROOMS];
for (int i = 0;
i < g->room_count;
i++) dist[i] = -1;
int stack[STRONGHOLD_MAX_ROOMS], sp = 0;
dist[0] = 0;
stack[sp++] = 0;
while (sp > 0) {
        int cur = stack[--sp];
        for (int e = 0; e < g->edge_count; e++) {
            int nb = -1;
            if (g->edges[e].a == cur) nb = g->edges[e].b;
            else if (g->edges[e].b == cur) nb = g->edges[e].a;
            if (nb < 0 || dist[nb] >= 0) continue;
            dist[nb] = dist[cur] + 1;
            stack[sp++] = nb;
        }
    }
    return dist[target];
}

// how many edges touch a room. degree-1 rooms are dead-ends, the good spots
// for the portal and for libraries.
static int room_degree(const stronghold_graph *g, int idx) {
    int n = 0;
    for (int e = 0; e < g->edge_count; e++)
        if (g->edges[e].a == idx || g->edges[e].b == idx) n++;
    return n;
}

void stronghold_graph_assign_types(stronghold_graph *g, const stronghold_config *cfg,
                                   stronghold_rng *rng) {
    if (g->room_count == 0) return;
// portal: the dead-end farthest (by graph distance) from the spawn hub.
int best = -1, best_depth = -1, best_fallback = -1, fb_depth = -1;
for (int i = 1;
i < g->room_count;
i++) {
        int depth = graph_depth_of(g, i);
        if (depth > fb_depth) { fb_depth = depth; best_fallback = i; }
        if (room_degree(g, i) <= 1 && depth > best_depth) {
            best_depth = depth; best = i;
        }
    }
    if (best < 0) best = best_fallback >= 0 ? best_fallback : 0;
g->portal_room = best;
g->rooms[best].type = STRONGHOLD_ROOM_PORTAL;
{
        int cx, cy, cz;
        stronghold_box_center(&g->rooms[best].box, &cx, &cy, &cz);
        g->rooms[best].box = stronghold_box_at(cx - PORTAL_W / 2, g->rooms[best].box.y0,
                                               cz - PORTAL_W / 2, PORTAL_W, PORTAL_H, PORTAL_W);
    }

    // type the rest. dead-ends prefer libraries;
through-rooms can be prisons
    // or junctions. the hub (room 0) and the portal stay as-is.
    for (int i = 0;
i < g->room_count;
i++) {
        if (i == g->portal_room || i == 0) continue;
        int deg = room_degree(g, i);
        if (deg <= 1 && stronghold_rng_chance(rng, cfg->library_chance)) {
            g->rooms[i].type = STRONGHOLD_ROOM_LIBRARY;
        } else if (deg >= 3) {
            g->rooms[i].type = STRONGHOLD_ROOM_JUNCTION;
        } else if (stronghold_rng_chance(rng, cfg->prison_chance)) {
            g->rooms[i].type = STRONGHOLD_ROOM_PRISON;
        } else {
            g->rooms[i].type = STRONGHOLD_ROOM_HALL;
        }
    }

    // tag stairwell rooms: any room reached by a stair edge gets the type so
    // the carver knows to drop a ladder shaft on that wall.
    for (int e = 0;
e < g->edge_count;
e++) {
        if (!g->edges[e].stair) continue;
        int lo = g->edges[e].b;   // dst is always the deeper one in grow()
        if (g->rooms[lo].type == STRONGHOLD_ROOM_HALL)
            g->rooms[lo].type = STRONGHOLD_ROOM_STAIRWELL;
    }
}
