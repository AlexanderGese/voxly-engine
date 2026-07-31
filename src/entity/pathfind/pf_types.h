#ifndef ENTITY_PATHFIND_PF_TYPES_H
#define ENTITY_PATHFIND_PF_TYPES_H
#include "../../math/vec3.h"
#include <stdint.h>
#define PF_WINDOW_RADIUS   24
#define PF_WINDOW_SIZE     (PF_WINDOW_RADIUS * 2 + 1)
#define PF_MAX_STEP_UP      1
#define PF_MAX_STEP_DOWN    3
#define PF_MAX_NODES       4096
#define PF_MAX_WAYPOINTS    96
typedef struct {
    int16_t x, y, z;
} pf_coord;
typedef struct {
    vec3 pos;
} pf_waypoint;
typedef struct {
    pf_waypoint pts[PF_MAX_WAYPOINTS];
    int         count;
    int         cursor;    // which waypoint we're walking toward
    int         found;     // 1 = real path, 0 = gave up / no path
} pf_path;
static inline pf_coord pf_coord_make(int x, int y, int z) {
    pf_coord c = { (int16_t)x, (int16_t)y, (int16_t)z };
    return c;
}

static inline int pf_coord_eq(pf_coord a, pf_coord b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

// pack a coord into a u64 for hashmap keys. y is small so 16 bits is plenty.
static inline uint64_t pf_coord_key(pf_coord c) {
    uint64_t x = (uint64_t)(uint16_t)c.x;
    uint64_t y = (uint64_t)(uint16_t)c.y;
    uint64_t z = (uint64_t)(uint16_t)c.z;
    return x | (y << 16) | (z << 32);
}
#endif
