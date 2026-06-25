#ifndef WORLD_STRONGHOLD_GRAPH_H
#define WORLD_STRONGHOLD_GRAPH_H
#include "stronghold_types.h"
#include "stronghold_box.h"
#include "stronghold_rand.h"
// the room graph. this is the heart of the generator. we grow a graph of typed
// rooms by a guided random walk: pick an open room, pick a free side, project a
// corridor stub + a new room footprint, reject on overlap, repeat until we hit
// the room budget or run out of frontier. then we type the rooms (one portal,
#define STRONGHOLD_MAX_ROOMS   64
#define STRONGHOLD_MAX_EDGES   96
#define STRONGHOLD_ROOM_DOORS   6   // doors stored per room
typedef struct {
    stronghold_box       box;        // world-space footprint (carve target)
    stronghold_room_type type;
    int                  level;      // 0 = top floor, grows downward
    uint32_t             seed;       // per-room derived seed
    int                  open_sides; // bitmask of dirs still free for an edge
    stronghold_door      doors[STRONGHOLD_ROOM_DOORS];
    int                  door_count;
} stronghold_room;
typedef struct {
    int            a, b;       // room indices
    stronghold_dir side_a;     // dir the corridor leaves room a
    int            length;     // axial corridor length in blocks
    int            stair;      // 1 if the corridor steps down a level
} stronghold_edge;
typedef struct {
    stronghold_room  rooms[STRONGHOLD_MAX_ROOMS];
    int              room_count;
    stronghold_edge  edges[STRONGHOLD_MAX_EDGES];
    int              edge_count;
    int              portal_room;    // index of the one portal room, or -1
    stronghold_box   bounds;         // running union of all footprints
} stronghold_graph;
void stronghold_graph_init(stronghold_graph *g);
int  stronghold_graph_grow(stronghold_graph *g, const stronghold_config *cfg,
                           int ox, int oy, int oz, stronghold_rng *rng);
void stronghold_graph_assign_types(stronghold_graph *g, const stronghold_config *cfg,
                                   stronghold_rng *rng);
void stronghold_room_add_door(stronghold_room *r, int x, int y, int z,
                              stronghold_dir facing, int gated);
#endif
