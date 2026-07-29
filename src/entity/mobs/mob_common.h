#ifndef ENTITY_MOBS_MOB_COMMON_H
#define ENTITY_MOBS_MOB_COMMON_H

#include "../../math/vec3.h"
#include <stdint.h>
#include <stdbool.h>

// self contained mob struct for the new mobs/ modules.
// kept separate from the engine `entity` so these files link clean on
// their own. the game glue can copy pos/hp back into an entity later.
//
// id 0 means "no mob".

// kind of creature. hostile ones chase the player, passive ones wander/flee.
typedef enum {
    VOXL_MOB_NONE = 0,
    VOXL_MOB_ZOMBIE,
    VOXL_MOB_SKELETON,
    VOXL_MOB_CREEPER,
    VOXL_MOB_SPIDER,
    VOXL_MOB_COW,
    VOXL_MOB_PIG,
    VOXL_MOB_CHICKEN,
    VOXL_MOB_SHEEP,
    VOXL_MOB_KIND_COUNT
} voxl_mob_kind;

// tiny state machine shared by every mob. not every mob uses every state.
typedef enum {
    VOXL_MS_IDLE = 0,
    VOXL_MS_WANDER,
    VOXL_MS_CHASE,
    VOXL_MS_ATTACK,
    VOXL_MS_FLEE,
    VOXL_MS_HURT,
    VOXL_MS_DEAD,
    VOXL_MS_COUNT
} voxl_mob_state;

struct voxl_mob {
    uint32_t        id;
    voxl_mob_kind   kind;
    voxl_mob_state  state;

    vec3            pos;
    vec3            vel;
    float           yaw;        // radians, 0 = facing -z

    int             health;
    int             max_health;
    int             on_ground;

    // timers (seconds). reused per state, meaning depends on state.
    float           state_timer;
    float           hurt_timer;
    float           attack_cd;   // cooldown until next swing

    // who we are angry at. 0 = nobody / player is implicit.
    uint32_t        target_id;

    // steering scratch: a point we are currently steering toward.
    vec3            wander_target;
    int             has_wander_target;
};

// short alias used throughout the mobs modules.
typedef struct voxl_mob voxl_mob;

#endif
