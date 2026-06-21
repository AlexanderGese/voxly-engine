#ifndef WORLD_MINESHAFT_TYPES_H
#define WORLD_MINESHAFT_TYPES_H

#include <stdint.h>
#include "../block.h"
#include "../block_ext.h"

// plain-data types shared across the abandoned-mineshaft generator. same shape
// the rest of worldgen uses: pieces emit voxels into a buffer, the driver stamps
// whatever lands inside a chunk. stays chunk-free and render-free so the link
// graph stays a dag and unit tests can run the layout without a gl context.

// what a piece resolved to. drives which builder runs and how supports/rails
// get laid down. corridors are the spine, rooms are the rare wide intersections.
typedef enum {
    MINESHAFT_PIECE_NONE = 0,
    MINESHAFT_PIECE_CORRIDOR,   // 3-wide tunnel with plank supports
    MINESHAFT_PIECE_ROOM,       // wide intersection / ore cache room
    MINESHAFT_PIECE_SHAFT,      // vertical drop with a ladder
    MINESHAFT_PIECE_TYPE_COUNT
} mineshaft_piece_type;

// cardinal facing. plain ints so we can rotate by adding mod 4. matches the
// convention the structgen pieces use (north = -z, clockwise from there).
typedef enum {
    MINESHAFT_NORTH = 0,   // -z
    MINESHAFT_EAST  = 1,   // +x
    MINESHAFT_SOUTH = 2,   // +z
    MINESHAFT_WEST  = 3    // -x
} mineshaft_dir;

// a single placed voxel in world space. same layout as structgen_voxel on
// purpose; callers already speaking that dialect can splice without translating.
typedef struct {
    int x, y, z;
    block_id id;
} mineshaft_voxel;

// integer box in world space, inclusive min, exclusive max. used for corridor
// footprints, room reservation and overlap rejection on the maze grid.
typedef struct {
    int x0, y0, z0;   // min corner
    int x1, y1, z1;   // max corner (exclusive)
} mineshaft_box;

// one mineshaft is rooted at a chunk-derived anchor and lives at a fixed depth
// band. y is resolved by the driver from the host heightmap so the shaft tucks
// itself well under the surface instead of poking out of a hill.
typedef struct {
    int      anchor_x, anchor_z;   // world xz the maze is rooted at
    int      floor_y;              // y the corridor floors sit on
    uint32_t seed;                 // per-mineshaft derived seed
} mineshaft_site;

// tunables for the whole subsystem. one config the driver owns. the block ids
// live here too so a host with a different atlas can swap the palette without
// touching the builders (there is no dedicated cobweb block, so we borrow one).
typedef struct {
    int   region_size;       // chunks per placement region (grid spacing)
    float spawn_chance;      // per eligible region, [0,1]
    int   depth_min;         // lowest floor_y the shaft is allowed to reach
    int   depth_below;       // how far under the surface the top floor sits
    int   max_pieces;        // hard cap on corridors+rooms in one maze
    int   corridor_len;      // blocks per corridor segment
    int   max_voxels;        // hard cap on a single mineshaft's voxels

    // palette. pulled into config so the dry stuff stays out of the builders.
    block_id mat_support;    // vertical support posts (fence)
    block_id mat_beam;       // plank crossbeams / floor planks
    block_id mat_fill;       // wall backing where we carved into solid
    block_id mat_rail;       // floor "rail" marker block
    block_id mat_web;        // cobweb stand-in (plant-ish, non-solid)
    block_id mat_torch;      // sparse lighting
    block_id mat_chest;      // ore-room loot marker
    block_id ore_common;     // walls of ore rooms
    block_id ore_rare;       // sparse richer ore
} mineshaft_config;

mineshaft_config mineshaft_config_default(void);

#endif
