#ifndef PLAYER_FISHING_TYPES_H
#define PLAYER_FISHING_TYPES_H

#include <stdint.h>
#include "../../math/vec3.h"
#include "../../world/block.h"

// shared vocabulary for the fishing subsystem. one header so the rest of the
// module doesn't grow a thicket of cross includes (same deal as tools_types).

// where a cast can possibly be. derived from what's under the bobber when it
// lands. open water fishes best; the others are penalised or outright dead.
typedef enum {
    WATER_NONE = 0,    // no water at all, line just lies there
    WATER_OPEN,        // a tidy patch of water, the good case
    WATER_OBSTRUCTED,  // water but boxed in by blocks, slower bites
    WATER_KIND_COUNT
} fishing_water_kind;

// top level state of a cast. the session machine walks these in order and
// loops back to IDLE when the player reels in or the line snaps.
typedef enum {
    CAST_IDLE = 0,     // nothing in the water
    CAST_FLYING,       // bobber arcing out, not landed yet
    CAST_WAITING,      // floating, counting down to a bite
    CAST_LURING,       // approach particles, bite is imminent
    CAST_BITING,       // the brief window where reeling lands a catch
    CAST_HOOKED,       // something is on, player is fighting the line
    CAST_REELING,      // pulling the catch back toward the rod
    CAST_SNAPPED,      // tension blew the line, catch lost
    CAST_STATE_COUNT
} fishing_cast_state;

// what a single loot roll produced. category drives presentation and which
// table we rolled from; block is the actual item id handed to the world.
typedef enum {
    CATCH_NONE = 0,
    CATCH_FISH,        // the usual; food
    CATCH_TREASURE,    // rare, the good stuff
    CATCH_JUNK,        // boots, sticks, sadness
    CATCH_CATEGORY_COUNT
} fishing_catch_category;

// one entry in a weighted loot table. weight is relative within its table.
// min/max give a stack size; most entries are 1..1 but treasure can vary.
typedef struct {
    block_id               block;
    fishing_catch_category category;
    int                    weight;
    int                    min_count;
    int                    max_count;
} fishing_loot_entry;

// a resolved catch, the thing reel-in actually awards.
typedef struct {
    block_id               block;
    fishing_catch_category category;
    int                    count;
} fishing_catch;

// the bobber: a tiny floating body. lives in world space, bobs on water and
// gets yanked toward the rod during a reel. owns no rendering, just the sim.
typedef struct {
    vec3   pos;
    vec3   vel;
    int    landed;         // has it hit water/ground yet
    fishing_water_kind water;
    float  submerge;       // 0 floating high, 1 fully dunked. cosmetic + bite cue
    float  bob_phase;      // accumulates so the idle bob isn't perfectly static
} fishing_bobber;

// rod stats after enchants are folded in. lure shortens the wait, luck of the
// sea shifts the table toward treasure and away from junk.
typedef struct {
    int    lure;           // 0..3, each level trims wait time
    int    luck;           // 0..3, biases the loot roll
    float  max_tension;    // line strength; heavier catches risk a snap
    float  cast_power;     // launch speed scalar for the arc
} fishing_rod;

#endif
