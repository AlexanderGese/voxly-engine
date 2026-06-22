#ifndef WORLD_OREGEN_TYPES_H
#define WORLD_OREGEN_TYPES_H

#include <stdint.h>
#include "../block.h"

// shared plain-data types for the ore generator. numbers in, numbers out.
// nothing in here touches chunks or render so the worldgen driver can stamp
// the results wherever it likes. kept separate from the gen2 ore table on
// purpose: that one is height-banded value tweaks, this one grows actual
// 3d blobs with density curves. they can coexist or one can replace the
// other later, im not married to either.

// how an ore likes to be distributed vertically. the curve is sampled per
// y and turned into a spawn weight, so you get nice falloff at band edges
// instead of a hard cutoff.
typedef enum {
    OREGEN_CURVE_UNIFORM = 0, // flat across the band
    OREGEN_CURVE_TRIANGLE,    // peak in the middle, linear falloff
    OREGEN_CURVE_BOTTOM,      // densest near the floor, fades up
    OREGEN_CURVE_TOP,         // densest near the ceiling, fades down
    OREGEN_CURVE_COUNT
} oregen_curve;

// the shape a single vein takes once seeded. cheap to evaluate, all of them
// boil down to "is this voxel inside the blob".
typedef enum {
    OREGEN_SHAPE_BLOB = 0,    // fuzzy ellipsoid cluster, the common case
    OREGEN_SHAPE_VEIN,        // thin wandering worm, for stringy deposits
    OREGEN_SHAPE_POCKET,      // tight near-spherical lump
    OREGEN_SHAPE_COUNT
} oregen_shape;

// static description of one ore kind. this is the table the rest of the
// module reads. y units are world space.
typedef struct {
    const char  *name;
    block_id     block;       // what we actually place
    int          y_min, y_max;// inclusive spawn band
    oregen_curve curve;       // vertical density shape inside the band
    oregen_shape shape;
    float        tries_per_chunk; // expected vein seeds in a 16x16 column
    int          size_min, size_max; // target voxel count per vein
    float        squish;      // <1 squashes vertically, veins lie flat-ish
} oregen_ore;

// a seeded but not-yet-expanded vein. produced by the seeder, consumed by
// the blob/vein builders. world-space center plus rolled parameters.
typedef struct {
    int          kind;        // index into the ore table
    int          cx, cy, cz;  // world-space center
    int          size;        // rolled target voxel count
    float        radius;      // rolled base radius (blob/pocket)
    oregen_shape shape;
    block_id     block;
    uint32_t     seed;        // per-vein derived seed
} oregen_vein;

// one emitted voxel. driver stamps these into the world.
typedef struct {
    int      x, y, z;
    block_id id;
} oregen_cell;

#endif
