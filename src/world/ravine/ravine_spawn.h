#ifndef WORLD_RAVINE_SPAWN_H
#define WORLD_RAVINE_SPAWN_H

#include <stdint.h>
#include "ravine_types.h"
#include "ravine_path.h"

// ravine spawning. ravines are big and rare, so we cant decide "does this chunk
// have one" by looking at the chunk alone — the ravine almost certainly started
// in a different chunk. instead we tile the world into coarse spawn regions (a
// handful of chunks on a side), roll each region once against rarity for a
// ravine anchored at a jittered world point inside it, and let a chunk pull in
// every nearby region's ravine whose reach could cross into it. deterministic
// from world coords + seed, so neighbouring chunks always agree on the curve.

#define RAVINE_REGION_CHUNKS  4   // a spawn region is this many chunks per side
#define RAVINE_REGION_BLOCKS  (RAVINE_REGION_CHUNKS * CHUNK_SIZE_X)

// a spawned ravine anchor before its path is built.
typedef struct {
    int      region_x, region_z;  // which spawn region it belongs to
    float    anchor_x;            // jittered world xz of the ravine head
    float    anchor_z;
    uint32_t stream;              // private rng seed for this ravine
} ravine_anchor;

// does spawn region (region_x, region_z) roll a ravine? if so fills *out and
// returns 1. uses rarity + the hashed region coords.
int ravine_spawn_region(const ravine_params *p, int region_x, int region_z,
                        ravine_anchor *out);

// collect every ravine anchor whose reach might touch chunk (cx,cz). writes up
// to max anchors into out, returns the count. scans the 3x3 block of spawn
// regions around the chunk, which covers a ravine's bounded length.
int ravine_spawn_collect(const ravine_params *p, int cx, int cz,
                         ravine_anchor *out, int max);

#endif
