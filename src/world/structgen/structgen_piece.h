#ifndef WORLD_STRUCTGEN_PIECE_H
#define WORLD_STRUCTGEN_PIECE_H

#include "structgen_types.h"
#include "structgen_buffer.h"
#include "structgen_box.h"
#include "structgen_rand.h"

// a "piece" is one placed building/room. piece assembly works by laying down
// pieces relative to a structure origin, tracking their footprints so nothing
// overlaps, then asking each to stamp its voxels. think jigsaw-from-pieces but
// way simpler: we place onto a flat plan, not a recursive tree.

// what a piece is, drives which builder runs.
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

// a plan is the list of pieces chosen for one structure plus the bookkeeping
// used to reject overlaps while we lay them down.
typedef struct {
    structgen_piece pieces[64];
    int count;
    structgen_box   bounds;      // running union of all footprints
} structgen_plan;

void structgen_plan_init(structgen_plan *p, int origin_x, int origin_y, int origin_z);

// try to add a piece. returns 1 if it fit (no overlap, room left), else 0.
// the new piece's footprint is in final world coords already.
int  structgen_plan_try_add(structgen_plan *p, structgen_piece_type type,
                            structgen_box footprint, structgen_dir facing,
                            uint32_t seed);

// stamp every piece in the plan into out. returns total voxels emitted.
int  structgen_plan_build(const structgen_plan *p, structgen_buffer *out);

// build a single piece. dispatched by type. exposed for testing / reuse.
int  structgen_piece_build(const structgen_piece *pc, structgen_buffer *out);

#endif
