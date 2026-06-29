#ifndef WORLD_STRUCTGEN_TYPES_H
#define WORLD_STRUCTGEN_TYPES_H

#include <stdint.h>
#include "../block.h"

// plain-data types shared across the structure generator. like gen2 this
// stays chunk-free and render-free: structures emit voxels into a buffer
// and the worldgen driver stamps them wherever it wants. keeps link clean.

// which kind of structure a placement slot resolved to.
typedef enum {
    STRUCTGEN_NONE = 0,
    STRUCTGEN_VILLAGE,
    STRUCTGEN_DUNGEON,
    STRUCTGEN_RUIN,
    STRUCTGEN_KIND_COUNT
} structgen_kind;

// cardinal facing. used for piece orientation and door/connector sides.
// kept as plain ints so we can rotate by adding mod 4.
typedef enum {
    STRUCTGEN_NORTH = 0,   // -z
    STRUCTGEN_EAST  = 1,   // +x
    STRUCTGEN_SOUTH = 2,   // +z
    STRUCTGEN_WEST  = 3    // -x
} structgen_dir;

// a single placed voxel in world space. mirrors gen2_placement on purpose
// so callers that already speak gen2 can splice the two without translating.
typedef struct {
    int x, y, z;
    block_id id;
} structgen_voxel;

// integer box in world space, inclusive min, exclusive max. structures use
// this for footprint reservation and overlap rejection.
typedef struct {
    int x0, y0, z0;   // min corner
    int x1, y1, z1;   // max corner (exclusive)
} structgen_box;

// a structure occupies a region centered on a "anchor" world position.
// y is filled in later by the driver once it knows the surface height.
typedef struct {
    structgen_kind kind;
    int   anchor_x, anchor_z;   // world xz the structure is rooted at
    int   ground_y;             // surface y under the anchor (driver fills)
    uint32_t seed;              // derived per-structure seed
} structgen_site;

// tunables for the whole subsystem. one global-ish config the driver owns.
typedef struct {
    int   sea_level;
    int   region_size;          // chunks per placement region (grid spacing)
    int   village_per_region;   // 0/1, attempts per region
    float dungeon_chance;       // per eligible chunk, [0,1]
    float ruin_chance;          // per eligible chunk, [0,1]
    int   max_voxels;           // hard cap on a single structure
} structgen_config;

structgen_config structgen_config_default(void);

#endif
