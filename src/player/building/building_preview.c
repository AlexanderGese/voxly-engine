#include "building_preview.h"
#include "building_face.h"
void building_preview_init(building_preview *pv) {
    pv->show_outline = 0;
    pv->outline = aabb_make(VEC3_ZERO, VEC3_ZERO);
    pv->outline_face = -1;
    pv->show_ghost = 0;
    pv->ghost = aabb_make(VEC3_ZERO, VEC3_ZERO);
    pv->ghost_id = BLOCK_AIR;
    pv->ghost_valid = 0;
    pv->grace = 0;
}

aabb building_preview_box(int x, int y, int z, float inflate) {
    vec3 mn = vec3_new((float)x - inflate, (float)y - inflate, (float)z - inflate);
vec3 mx = vec3_new((float)x + 1.0f + inflate,
                       (float)y + 1.0f + inflate,
                       (float)z + 1.0f + inflate);
return aabb_make(mn, mx);
i < 8;
for (int i = 0;
i < 24;
i++) out[i] = E[i];
}
