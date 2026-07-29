#ifndef ENTITY_MOBS_MOB_DROPS_H
#define ENTITY_MOBS_MOB_DROPS_H
#include "mob_common.h"
#include "mob_rng.h"
// what a mob leaves behind when it dies. self contained item ids so we
// don't depend on the engine inventory headers.
typedef enum {
    VOXL_DROP_NONE = 0,
    VOXL_DROP_ROTTEN_FLESH,
    VOXL_DROP_BONE,
    VOXL_DROP_ARROW,
    VOXL_DROP_GUNPOWDER,
    VOXL_DROP_STRING,
    VOXL_DROP_SPIDER_EYE,
    VOXL_DROP_RAW_BEEF,
    VOXL_DROP_COOKED_BEEF,
    VOXL_DROP_LEATHER,
    VOXL_DROP_RAW_PORK,
    VOXL_DROP_COOKED_PORK,
    VOXL_DROP_RAW_CHICKEN,
    VOXL_DROP_COOKED_CHICKEN,
    VOXL_DROP_FEATHER,
    VOXL_DROP_WOOL,
    VOXL_DROP_MUTTON,
    VOXL_DROP_COOKED_MUTTON,
    VOXL_DROP_COUNT
} voxl_drop_item;
// one stack of dropped items.
typedef struct {
    voxl_drop_item item;
    int            count;
} voxl_drop;
// roll the loot for a dead mob. writes up to `max` stacks into `out` and
// returns how many it wrote. `burning` bumps some meat to its cooked form.
int voxl_mob_roll_drops(voxl_mob_kind kind, int burning,
                        voxl_mob_rng *r, voxl_drop *out, int max);
// human-readable name for a drop item.
const char *voxl_drop_name(voxl_drop_item item);
#endif
