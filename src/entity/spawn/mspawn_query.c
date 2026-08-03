#include "mspawn_query.h"
#include "../../world/biome.h"
#include "../../world/worldgen.h"

biome_id mspawn_query_biome(world *w, int wx, int wz) {
    float t, h;
    biome_sample(wx, wz, w->seed, &t, &h);
    return biome_pick(t, h);
}

int mspawn_query_surface(world *w, int wx, int wz) {
    return worldgen_height_at(wx, wz, w->seed);
}

int mspawn_query_category_open(mspawn_category cat, float day_hour) {
    int is_day = (day_hour >= 6.0f && day_hour < 18.0f);
    switch (cat) {
        case MSPAWN_CAT_PASSIVE: return is_day;   // grazers in daylight
        case MSPAWN_CAT_HOSTILE: return !is_day;  // monsters at night
        case MSPAWN_CAT_AMBIENT: return 1;        // whenever theres budget
        default:                 return 0;
    }
}
