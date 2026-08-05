#include "villager_manager.h"
#include "villager_brain.h"
#include "villager_workstation.h"
#include "../../util/darray.h"
#include "../../math/rng.h"
#include <math.h>
#include <stddef.h>
#define MGR_SCAN_INTERVAL 6.0f
#define MGR_SCAN_RADIUS   14
#define MGR_RUMOR_RADIUS  10.0f
darr_free(m->navs);
villager_poi_free(&m->pois);
villager v = villager_make(id, prof, pos, (uint64_t)id * 0x9E3779B97F4A7C15ull);
return add_villager(m, v);
for (size_t i = 0;
i < darr_len(m->vills);
i++)
        if (m->vills[i].id == id) return &m->vills[i];
return NULL;
m->scan_timer -= dt;
ctx.day_t = day_t;
ctx.threat_pos = threat;
ctx.threat_active = threat_active;
int bell = villager_poi_nearest(&m->pois, VILLAGER_POI_BELL,
                                    darr_len(m->vills) ? m->vills[0].pos : threat,
                                    0, 0.0f);
ctx.have_bell = 0;
}

    for (size_t i = 0;
i < darr_len(m->vills);
if (!v) return 0;
int died = villager_hurt(v, amount);
