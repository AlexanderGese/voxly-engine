#ifndef ENTITY_VILLAGER_MANAGER_H
#define ENTITY_VILLAGER_MANAGER_H

#include "villager.h"
#include "villager_pathing.h"
#include "villager_poi.h"
#include "../../world/world.h"
#include "../../math/vec3.h"

// owns the whole village: the villager roster, their navigation state, and
// the shared poi registry. one of these per world. handles id allocation,
// spawning, the per-tick update loop, and the player-facing trade hook.

typedef struct {
    villager      *vills;    // darray, parallel to navs
    villager_nav  *navs;     // darray, 1:1 with vills
    villager_poi_set pois;
    uint32_t       next_id;
    float          scan_timer;   // throttles the world poi rescan
    int            poi_origin_x; // last scan center, to avoid rescanning still
    int            poi_origin_z;
} villager_manager;

void villager_manager_init(villager_manager *m);
void villager_manager_free(villager_manager *m);

// spawn an adult or baby; returns the new villager's id (0 on failure).
uint32_t villager_manager_spawn(villager_manager *m, villager_profession prof, vec3 pos);
uint32_t villager_manager_spawn_baby(villager_manager *m, vec3 pos);

// look up a villager by id. NULL if gone. index-stable within a frame only.
villager *villager_manager_get(villager_manager *m, uint32_t id);

// tick every villager. `day_t` is the normalized day clock; `threat` is an
// optional scary position (player_pos when aggro'd or a nearby mob); pass
// threat_active 0 to mean "all calm".
void villager_manager_tick(villager_manager *m, world *w, float day_t,
                           vec3 threat, int threat_active, float dt);

// damage a villager (e.g. player hit it). spreads a negative-gossip rumor to
// nearby villagers and flips the victim to panic. returns 1 if it died.
int villager_manager_hurt(villager_manager *m, uint32_t id, int amount, vec3 attacker);

// player attempts a trade with villager `id`, slot `slot`. on success returns
// 1 and fills out the received item/count and the price paid.
int villager_manager_trade(villager_manager *m, uint32_t id, int slot,
                           int player_has, block_id *out_item,
                           int *out_count, int *paid);

int villager_manager_count(const villager_manager *m);

#endif
