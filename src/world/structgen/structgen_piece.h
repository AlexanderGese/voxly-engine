#ifndef WORLD_STRUCTGEN_PIECE_H
#define WORLD_STRUCTGEN_PIECE_H
#include "structgen_types.h"
#include "structgen_buffer.h"
#include "structgen_box.h"
#include "structgen_rand.h"
typedef enum {
    PIECE_HOUSE = 0,    // village dwelling
    PIECE_FARM,         // tilled plot, low fence
    PIECE_WELL,         // village centerpiece
    PIECE_ROOM,         // dungeon room
    PIECE_CORRIDOR,     // dungeon connector
    PIECE_PILLAR_HALL,  // ruin colonnade
    PIECE_TYPE_COUNT
} structgen_piece_type;
typedef struct {
    structgen_piece_type type;
    structgen_box   footprint;   // world-space, y filled at place time
    structgen_dir   facing;      // door/entrance side
    uint32_t        seed;        // per-piece derived seed
} structgen_piece;
typedef struct {
    structgen_piece pieces[64];
    int count;
    structgen_box   bounds;      // running union of all footprints
} structgen_plan;
void structgen_plan_init(structgen_plan *p, int origin_x, int origin_y, int origin_z);
int  structgen_plan_try_add(structgen_plan *p, structgen_piece_type type,
                            structgen_box footprint, structgen_dir facing,
                            uint32_t seed);
#endif
