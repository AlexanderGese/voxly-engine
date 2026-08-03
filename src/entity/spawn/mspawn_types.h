#ifndef ENTITY_SPAWN_MSPAWN_TYPES_H
#define ENTITY_SPAWN_MSPAWN_TYPES_H

#include <stdint.h>
#include "../../math/vec3.h"
#include "../../world/biome.h"
#include "../entity.h"

// shared plain-data types for the mob spawn driver. nothing in here pokes the
// world or the mob registry directly; the candidate finders and the rule table
// produce numbers and the driver decides what to do with them. kept apart from
// the older entity/spawner.c on purpose so the two prefixes never clash while i
// rip the old one out piece by piece.

// what time-of-day window a mob is allowed to appear in. dusk/dawn matter
// because the day-night curve crosses the hostile threshold there and i wanted
// crepuscular critters (bats, that sort of thing) without special-casing them
// in the driver.
typedef enum {
    MSPAWN_TIME_ANY = 0,   // doesnt care about the clock
    MSPAWN_TIME_DAY,       // sun is up
    MSPAWN_TIME_NIGHT,     // sun is down, the dangerous window
    MSPAWN_TIME_DUSK,      // narrow band around sunset/sunrise
    MSPAWN_TIME_COUNT
} mspawn_time_band;

// the bucket a mob counts against for cap purposes. hostiles and passives have
// separate ceilings; ambient (decorative) mobs get their own small budget so a
// field of cows cant starve out the bats and vice versa.
typedef enum {
    MSPAWN_CAT_PASSIVE = 0,
    MSPAWN_CAT_HOSTILE,
    MSPAWN_CAT_AMBIENT,
    MSPAWN_CAT_COUNT
} mspawn_category;

// where on the surface a mob needs to stand. most things want solid ground,
// but i leave room here for water/air spawners later without touching the
// ground scanner's contract.
typedef enum {
    MSPAWN_GROUND_SOLID = 0, // top of an opaque solid block, open air above
    MSPAWN_GROUND_WATER,     // submerged, in a water column
    MSPAWN_GROUND_AIR        // anywhere with headroom, cave dwellers
} mspawn_ground_kind;

// static description of one spawnable mob kind. this is the table the rest of
// the module reads. one row per entity_type we actually want to spawn; types
// absent from the table simply never spawn naturally.
typedef struct {
    entity_type        type;
    mspawn_category    category;
    mspawn_time_band   time_band;
    mspawn_ground_kind ground;
    int                min_light;     // lowest block+sun light it tolerates
    int                max_light;     // highest light it tolerates (hostiles low)
    int                y_min, y_max;   // inclusive world-y band
    int                pack_min;       // floor on pack size when chosen
    int                pack_max;       // ceiling on pack size
    float              weight;         // relative pick weight within a biome
} mspawn_entry;

// a fully-rolled spawn site: a world position plus the kind that wants it and
// the seed that produced it. the ground scanner fills position+light, the rule
// picker fills kind, the pack builder clones it around. cheap to copy.
typedef struct {
    vec3        pos;
    entity_type type;
    int         light;       // resolved light level at the site
    int         wy;          // floor y the mob stands on
    uint32_t    seed;        // per-site derived seed
} mspawn_site;

// rolled pack: a center site and a handful of member offsets. the driver walks
// these and asks the world whether each landing spot is actually clear before
// committing, so an offset can be silently dropped.
#define MSPAWN_PACK_MAX 8

typedef struct {
    mspawn_site center;
    vec3        offset[MSPAWN_PACK_MAX];
    int         count;       // valid entries in offset[]
} mspawn_pack;

#endif
