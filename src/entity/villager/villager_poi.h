#ifndef ENTITY_VILLAGER_POI_H
#define ENTITY_VILLAGER_POI_H

#include "villager_types.h"
#include "../../math/vec3.h"
#include <stdint.h>

// the point-of-interest registry. one per village (really per world, we
// keep it flat). villagers query this to find a bed, a free workstation, or
// the bell that anchors the gather point.
//
// claims are tracked by owner id so two villagers don't fight over one bed.
// id 0 means "unclaimed".

typedef struct {
    int               wx, wy, wz;
    uint8_t           kind;       // villager_poi_kind
    uint8_t           valid;      // 0 once the block is gone
    uint32_t          owner;      // villager id that claimed it, 0 = free
    float             ticket;     // soft cooldown before re-claim allowed
} villager_poi;

typedef struct {
    villager_poi *list;           // darray
} villager_poi_set;

void villager_poi_init(villager_poi_set *s);
void villager_poi_free(villager_poi_set *s);

// register a poi at a block. if one already exists there of the same kind
// it is reused (returns its index). returns -1 only on alloc failure.
int  villager_poi_add(villager_poi_set *s, villager_poi_kind kind,
                      int wx, int wy, int wz);

// drop a poi (block was broken). releases any owner. no-op if absent.
void villager_poi_remove(villager_poi_set *s, int wx, int wy, int wz);

// find the nearest *free* (or owned-by-`who`) poi of a kind within range.
// returns index or -1. range_sq is squared blocks; pass <=0 for unlimited.
int  villager_poi_nearest(villager_poi_set *s, villager_poi_kind kind,
                          vec3 from, uint32_t who, float range_sq);

// claim / release. claim fails (returns 0) if already owned by someone else.
int  villager_poi_claim(villager_poi_set *s, int index, uint32_t who);
void villager_poi_release(villager_poi_set *s, int index, uint32_t who);

// release every poi a villager holds (call on death / unload).
void villager_poi_release_all(villager_poi_set *s, uint32_t who);

// bleed ticket cooldowns. call once per tick with dt seconds.
void villager_poi_tick(villager_poi_set *s, float dt);

vec3 villager_poi_pos(const villager_poi_set *s, int index);

#endif
