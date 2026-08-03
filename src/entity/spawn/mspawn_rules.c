#include "mspawn_rules.h"
#include "../../config.h"
#include <stddef.h>
#define MSPAWN_NO_Y (-1)
static const mspawn_entry g_entries[] = {
    // type        category            time band          ground               minL maxL  ymin           ymax  pmin pmax weight
    { ET_COW,      MSPAWN_CAT_PASSIVE, MSPAWN_TIME_DAY,   MSPAWN_GROUND_SOLID,   9,  15, WORLD_SEA_LEVEL, MSPAWN_NO_Y, 2, 4, 1.0f },
    { ET_PIG,      MSPAWN_CAT_PASSIVE, MSPAWN_TIME_DAY,   MSPAWN_GROUND_SOLID,   9,  15, WORLD_SEA_LEVEL, MSPAWN_NO_Y, 2, 4, 1.0f },
    { ET_ZOMBIE,   MSPAWN_CAT_HOSTILE, MSPAWN_TIME_NIGHT, MSPAWN_GROUND_SOLID,   0,   7, MSPAWN_NO_Y,     MSPAWN_NO_Y, 1, 4, 1.0f },
    { ET_SKELETON, MSPAWN_CAT_HOSTILE, MSPAWN_TIME_NIGHT, MSPAWN_GROUND_SOLID,   0,   7, MSPAWN_NO_Y,     MSPAWN_NO_Y, 1, 3, 1.0f },
    { ET_SPIDER,   MSPAWN_CAT_HOSTILE, MSPAWN_TIME_NIGHT, MSPAWN_GROUND_SOLID,   0,   9, MSPAWN_NO_Y,     MSPAWN_NO_Y, 1, 2, 0.8f },
}
;
static const int g_entry_count = (int)(sizeof g_entries / sizeof g_entries[0]);
enum {
    E_COW = 0, E_PIG, E_ZOMBIE, E_SKELETON, E_SPIDER
}
;
static const int   plains_e[]    = { E_COW, E_PIG, E_ZOMBIE, E_SKELETON, E_SPIDER }
;
static const float plains_b[]    = { 1.4f,  1.0f,  1.0f,     0.9f,       0.7f }
;
static const int   forest_e[]    = { E_COW, E_PIG, E_ZOMBIE, E_SKELETON, E_SPIDER }
;
static const float forest_b[]    = { 1.0f,  1.2f,  1.0f,     0.8f,       1.1f }
;
static const int   desert_e[]    = { E_ZOMBIE, E_SKELETON }
;
static const float desert_b[]    = { 0.7f,     1.3f }
;
static const int   mountains_e[] = { E_COW, E_ZOMBIE, E_SKELETON }
;
static const float mountains_b[] = { 0.6f,  1.0f,     1.1f }
;
static const int   tundra_e[]    = { E_ZOMBIE, E_SKELETON }
;
static const float tundra_b[]    = { 1.0f,     1.0f }
;
static const int   swamp_e[]     = { E_PIG, E_ZOMBIE, E_SPIDER }
;
static const float swamp_b[]     = { 0.8f,  1.1f,     1.5f }
;
static const int   beach_e[]     = { E_ZOMBIE, E_SKELETON }
;
static const float beach_b[]     = { 0.9f,     1.0f }
;
#define ROSTER(e, b) { (e), (b), (int)(sizeof(e)/sizeof((e)[0])) }
static const mspawn_roster g_rosters[BIOME_COUNT] = {
    [BIOME_PLAINS]    = ROSTER(plains_e,    plains_b),
    [BIOME_FOREST]    = ROSTER(forest_e,    forest_b),
    [BIOME_DESERT]    = ROSTER(desert_e,    desert_b),
    [BIOME_MOUNTAINS] = ROSTER(mountains_e, mountains_b),
    [BIOME_TUNDRA]    = ROSTER(tundra_e,    tundra_b),
    [BIOME_SWAMP]     = ROSTER(swamp_e,     swamp_b),
    [BIOME_BEACH]     = ROSTER(beach_e,     beach_b),
}
;
const mspawn_entry *mspawn_entry_for(entity_type t) {
    for (int i = 0; i < g_entry_count; i++)
        if (g_entries[i].type == t) return &g_entries[i];
    return NULL;
}

const mspawn_entry *mspawn_entry_table(int *out_count) {
    if (out_count) *out_count = g_entry_count;
return g_entries;
}

mspawn_roster mspawn_roster_for(biome_id biome) {
    if (biome < 0 || biome >= BIOME_COUNT || g_rosters[biome].count == 0)
        return g_rosters[BIOME_PLAINS];
    return g_rosters[biome];
}

int mspawn_time_ok(const mspawn_entry *e, float day_hour) {
    // sun roughly up between 6 and 18. dusk is the two narrow bands either side
    // where the hostile-light threshold actually flips.
    int is_day  = (day_hour >= 6.0f && day_hour < 18.0f);
int is_dusk = (day_hour >= 17.5f && day_hour < 18.5f) ||
                  (day_hour >= 5.5f  && day_hour < 6.5f);
float total = 0.0f;
for (int i = 0;
i < ros.count;
float pick = mspawn_rng_f01(r) * total;
for (int i = 0;
i < ros.count;
i >= 0;
}
