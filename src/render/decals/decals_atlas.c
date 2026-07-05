#include "decals_atlas.h"
#include "decals_config.h"
#include "../../util/log.h"
#include <string.h>
static void tiles_to_uv(int tiles_x, int tiles_y,
                        int tx, int ty, int tw, int th,
                        float out0[2], float out1[2]) {
    float ix = 1.0f / (float)tiles_x;
    float iy = 1.0f / (float)tiles_y;
    out0[0] = (float)tx * ix;
    out0[1] = (float)ty * iy;
    out1[0] = (float)(tx + tw) * ix;
    out1[1] = (float)(ty + th) * iy;
}

void decals_atlas_init(decals_atlas *a) {
    memset(a, 0, sizeof *a);
a->tiles_x = DECALS_ATLAS_TILES_X;
a->tiles_y = DECALS_ATLAS_TILES_Y;
a->tex = 0;
a->count = 0;
}

int decals_atlas_find(const decals_atlas *a, const char *name) {
    for (int i = 0; i < a->count; i++)
        if (strncmp(a->entries[i].name, name, DECALS_ATLAS_NAME_LEN) == 0)
            return i;
    return -1;
}

int decals_atlas_add(decals_atlas *a, const char *name,
                     int tx, int ty, int tw, int th,
                     int has_normal, int ntx, int nty, uint16_t flags) {
    if (a->count >= DECALS_ATLAS_MAX_REGIONS) {
        LOGW("decals: atlas full, dropping region '%s'", name);
return -1;
return -1;
}

    decals_atlas_entry *e = &a->entries[a->count];
memset(e, 0, sizeof *e);
strncpy(e->name, name, DECALS_ATLAS_NAME_LEN - 1);
tiles_to_uv(a->tiles_x, a->tiles_y, tx, ty, tw, th,
                e->region.uv0, e->region.uv1);
the flag gates the actual blend.
        e->region.nuv0[0] = e->region.uv0[0];
e->region.nuv0[1] = e->region.uv0[1];
e->region.nuv1[0] = e->region.uv1[0];
e->region.nuv1[1] = e->region.uv1[1];
}
    e->flags = flags;
return a->count++;
return a->entries[idx].flags;
glBindTexture(GL_TEXTURE_2D, a->tex);
}

// the stock set. left column is albedo, right half of the page holds the
// matching normal tiles for the few that bother having them. coordinates are
// hand-placed to match the shipped decals.png;
