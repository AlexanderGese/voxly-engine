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
#endif
