#ifndef ENTITY_VILLAGER_MANAGER_H
#define ENTITY_VILLAGER_MANAGER_H
#include "villager.h"
#include "villager_pathing.h"
#include "villager_poi.h"
#include "../../world/world.h"
#include "../../math/vec3.h"
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
uint32_t villager_manager_spawn(villager_manager *m, villager_profession prof, vec3 pos);
uint32_t villager_manager_spawn_baby(villager_manager *m, vec3 pos);
villager *villager_manager_get(villager_manager *m, uint32_t id);
#endif
