#include "villager_workstation.h"
#include "villager_def.h"
#include "../../world/block.h"

#include <math.h>
#include <stddef.h>

// the engine has no bed/bell blocks, so we borrow: wood = a bed (you sleep
// on a wooden frame), torch = the bell (it's the village beacon). good enough
// for the gather/sleep behaviors to have somewhere to point at.
#define WS_BED_BLOCK   BLOCK_WOOD
#define WS_BELL_BLOCK  BLOCK_TORCH

// how far a villager will look for a job / bed from where it's standing.
#define WS_JOB_RANGE     12.0f
#define WS_BED_RANGE     16.0f

int villager_workstation_scan(villager_poi_set *pois, world *w,
                              vec3 origin, int radius) {
    int added = 0;
    int ox = (int)floorf(origin.x);
    int oy = (int)floorf(origin.y);
    int oz = (int)floorf(origin.z);

    // vertical band is narrower than horizontal — villages are flat-ish.
    for (int dx = -radius; dx <= radius; dx++) {
        for (int dz = -radius; dz <= radius; dz++) {
            for (int dy = -4; dy <= 4; dy++) {
                int wx = ox + dx, wy = oy + dy, wz = oz + dz;
                block_id b = world_get_block(w, wx, wy, wz);
                if (b == BLOCK_AIR) continue;

                if (b == WS_BED_BLOCK) {
                    int idx = villager_poi_add(pois, VILLAGER_POI_BED, wx, wy, wz);
                    if (idx >= 0) added++;
                    continue;
                }
                if (b == WS_BELL_BLOCK) {
                    int idx = villager_poi_add(pois, VILLAGER_POI_BELL, wx, wy, wz);
                    if (idx >= 0) added++;
                    continue;
                }
                // is this block a workstation any profession wants?
                villager_profession p = villager_def_for_block(b);
                if (p != VILLAGER_PROF_COUNT) {
                    int idx = villager_poi_add(pois, VILLAGER_POI_WORKSTATION,
                                               wx, wy, wz);
                    if (idx >= 0) added++;
                }
            }
        }
    }
    return added;
}

int villager_workstation_seek_job(villager *v, villager_poi_set *pois, vec3 from) {
    // nitwits and babies don't take jobs.
    if (v->prof == VILLAGER_PROF_NITWIT) return 0;
    if (v->is_baby) return 0;
    if (v->prof != VILLAGER_PROF_UNEMPLOYED) return 1;  // already employed

    float r2 = WS_JOB_RANGE * WS_JOB_RANGE;
    int idx = villager_poi_nearest(pois, VILLAGER_POI_WORKSTATION, from, v->id, r2);
    if (idx < 0) return 0;
    if (!villager_poi_claim(pois, idx, v->id)) return 0;

    // figure out which profession this station belongs to. we don't have the
    // block here, so we re-derive from the poi position via the registry; the
    // brain that has the world will set the real profession. as a safe
    // default, claim it and let validate() lock in the profession next pass.
    v->work_poi = idx;
    return 1;
}

int villager_workstation_validate(villager *v, villager_poi_set *pois, world *w) {
    if (v->work_poi < 0) return 0;

    vec3 p = villager_poi_pos(pois, v->work_poi);
    block_id b = world_get_block(w, (int)floorf(p.x), (int)floorf(p.y),
                                 (int)floorf(p.z));
    villager_profession want = villager_def_for_block(b);

    if (want == VILLAGER_PROF_COUNT) {
        // station's gone (mined, replaced). lose the job.
        villager_poi_release(pois, v->work_poi, v->id);
        villager_poi_remove(pois, (int)floorf(p.x), (int)floorf(p.y),
                            (int)floorf(p.z));
        v->work_poi = -1;
        villager_set_profession(v, VILLAGER_PROF_UNEMPLOYED);
        return 0;
    }

    // newly claimed and still unemployed? adopt the station's profession.
    if (v->prof == VILLAGER_PROF_UNEMPLOYED && want != VILLAGER_PROF_UNEMPLOYED)
        villager_set_profession(v, want);

    return 1;
}

int villager_workstation_claim_bed(villager *v, villager_poi_set *pois, vec3 from) {
    if (v->bed_poi >= 0) return 1;   // already have one
    float r2 = WS_BED_RANGE * WS_BED_RANGE;
    int idx = villager_poi_nearest(pois, VILLAGER_POI_BED, from, v->id, r2);
    if (idx < 0) return 0;
    if (!villager_poi_claim(pois, idx, v->id)) return 0;
    v->bed_poi = idx;
    return 1;
}
