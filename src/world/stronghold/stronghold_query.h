#ifndef WORLD_STRONGHOLD_QUERY_H
#define WORLD_STRONGHOLD_QUERY_H

#include "stronghold_graph.h"

// read-only queries over a built graph. the carve is done; this is the stuff a
// "locate stronghold" command, a minimap, or an ai pathing through the maze
// would lean on. nothing here mutates the graph or touches voxels.

// index of the room whose footprint contains (x,y,z), or -1 if none. linear
// scan; strongholds are small enough that an accel structure isnt worth it.
int stronghold_query_room_at(const stronghold_graph *g, int x, int y, int z);

// index of the room whose center is nearest to (x,y,z) by squared distance.
// returns -1 only for an empty graph.
int stronghold_query_nearest_room(const stronghold_graph *g, int x, int y, int z);

// straight-line squared distance from (x,y,z) to a room center. int math; the
// values stay well inside 32 bits for any sane stronghold extent.
long stronghold_query_room_dist_sq(const stronghold_graph *g, int room, int x, int y, int z);

// shortest hop count (edges) between two rooms over the corridor graph, or -1
// if disconnected / bad indices. bfs over the edge list.
int stronghold_query_hops(const stronghold_graph *g, int from, int to);

// fill `out_path` (cap entries) with the room indices on the shortest path from
// `from` to `to`, inclusive of both ends. returns the path length in rooms, or
// 0 if unreachable / no room / cap too small.
int stronghold_query_path(const stronghold_graph *g, int from, int to,
                          int *out_path, int cap);

// total interior air volume across all rooms (rough "size" metric for logs).
long stronghold_query_air_volume(const stronghold_graph *g);

// distance in rooms from the spawn hub (room 0) to the portal. handy as a
// difficulty readout. -1 if no portal / disconnected.
int stronghold_query_portal_depth(const stronghold_graph *g);

#endif
