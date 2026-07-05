#include "decals_atlas.h"
#include "decals_config.h"
#include "../../util/log.h"

#include <string.h>

// resolve tile (tx,ty) spanning (tw,th) into a [0,1] uv rect. half-texel inset
// would need the pixel size; we dont have it here and the atlas uses nearest
// sampling per region so a clean tile boundary is fine.
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
    }
    if (decals_atlas_find(a, name) >= 0) {
        LOGW("decals: duplicate atlas region '%s'", name);
        return -1;
    }
    // bounds-check so a fat-fingered tile coord doesnt sample outside the page
    if (tx < 0 || ty < 0 || tx + tw > a->tiles_x || ty + th > a->tiles_y) {
        LOGE("decals: region '%s' tiles out of atlas range", name);
        return -1;
    }

    decals_atlas_entry *e = &a->entries[a->count];
    memset(e, 0, sizeof *e);
    strncpy(e->name, name, DECALS_ATLAS_NAME_LEN - 1);
    tiles_to_uv(a->tiles_x, a->tiles_y, tx, ty, tw, th,
                e->region.uv0, e->region.uv1);

    if (has_normal) {
        tiles_to_uv(a->tiles_x, a->tiles_y, ntx, nty, tw, th,
                    e->region.nuv0, e->region.nuv1);
        flags |= DECALS_FLAG_NORMAL_MAP;
    } else {
        // point the normal rect at the albedo rect so the shader can sample
        // unconditionally without a branch; the flag gates the actual blend.
        e->region.nuv0[0] = e->region.uv0[0];
        e->region.nuv0[1] = e->region.uv0[1];
        e->region.nuv1[0] = e->region.uv1[0];
        e->region.nuv1[1] = e->region.uv1[1];
    }
    e->flags = flags;
    return a->count++;
}

const decals_atlas_region *decals_atlas_region_at(const decals_atlas *a, int idx) {
    if (idx < 0 || idx >= a->count) return NULL;
    return &a->entries[idx].region;
}

uint16_t decals_atlas_flags_at(const decals_atlas *a, int idx) {
    if (idx < 0 || idx >= a->count) return 0;
    return a->entries[idx].flags;
}

void decals_atlas_set_texture(decals_atlas *a, glid tex) {
    a->tex = tex;
}

void decals_atlas_bind(const decals_atlas *a, int unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, a->tex);
}

// the stock set. left column is albedo, right half of the page holds the
// matching normal tiles for the few that bother having them. coordinates are
// hand-placed to match the shipped decals.png; if you repaint the atlas, fix
// these too (theres no metadata file, sorry).
void decals_atlas_load_defaults(decals_atlas *a) {
    // scorch: big sooty splat, additive so it darkens then glows at the core
    decals_atlas_add(a, "scorch", 0, 0, 2, 2, 1, 4, 0,
                     DECALS_FLAG_ADDITIVE);
    // blood: plain albedo splat, fades normally
    decals_atlas_add(a, "blood", 2, 0, 1, 1, 0, 0, 0, 0);
    // boot: footprint, has a shallow normal so it dents mud a touch
    decals_atlas_add(a, "boot", 3, 0, 1, 1, 1, 6, 0, 0);
    // crack: spider-web fracture, normal-mapped, projects onto stone
    decals_atlas_add(a, "crack", 0, 2, 2, 2, 1, 4, 2,
                     DECALS_FLAG_NORMAL_MAP);
    // rune: glowing sigil, additive + world-locked (placed, not transient)
    decals_atlas_add(a, "rune", 2, 2, 1, 1, 0, 0, 0,
                     DECALS_FLAG_ADDITIVE | DECALS_FLAG_WORLD_LOCKED);
    // bullet: small hole, double-sided so it punches through thin slabs
    decals_atlas_add(a, "bullet", 3, 2, 1, 1, 1, 7, 2,
                     DECALS_FLAG_PROJECT_BACK);

    LOGI("decals: loaded %d default atlas regions", a->count);
}
