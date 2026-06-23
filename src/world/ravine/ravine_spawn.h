#ifndef WORLD_RAVINE_SPAWN_H
#define WORLD_RAVINE_SPAWN_H
#include <stdint.h>
#include "ravine_types.h"
#include "ravine_path.h"
#define RAVINE_REGION_CHUNKS  4   // a spawn region is this many chunks per side
#define RAVINE_REGION_BLOCKS  (RAVINE_REGION_CHUNKS * CHUNK_SIZE_X)
typedef struct {
    int      region_x, region_z;  // which spawn region it belongs to
    float    anchor_x;            // jittered world xz of the ravine head
    float    anchor_z;
    uint32_t stream;              // private rng seed for this ravine
} ravine_anchor;
#endif
