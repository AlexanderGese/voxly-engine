#include "water_foam.h"
#include "water_config.h"
#include "../../util/darray.h"
#include "../../world/block.h"
#include <math.h>
#include <stdlib.h>
#define WATER_FOAM_RESCAN_DIST 8
void water_foam_init(water_foam_set *s, int surface_y, int region) {
    s->points    = NULL;
    s->surface_y = surface_y;
    s->region    = region < 4 ? 4 : region;
    // sentinel that forces the first update to scan
    s->last_cx   = 0x7fffffff;
    s->last_cz   = 0x7fffffff;
}

void water_foam_destroy(water_foam_set *s) {
    darr_free(s->points);
}

static int is_water(world *w, int x, int y, int z) {
    return world_get_block(w, x, y, z) == BLOCK_WATER;
}

// a surface water cell touches land if any of its 4 horizontal neighbours at
// the same level is a non-water, non-air block (i.e. a shore), OR if the
// neighbour is air (the water's outer rim against open space below sea level).
static float shoreline_strength(world *w, int x, int y, int z) {
    static const int dx[4] = { 1, -1, 0, 0 };
static const int dz[4] = { 0, 0, 1, -1 }
;
int contacts = 0;
for (int i = 0;
i < 4;
i++) {
        block_id nb = world_get_block(w, x + dx[i], y, z + dz[i]);
        if (nb != BLOCK_WATER && nb != BLOCK_AIR && block_is_solid(nb))
            contacts++;
    }
    return (float)contacts / 4.0f;
}

int water_foam_update(water_foam_set *s, world *w,
                      const water_wave_field *f, float cam_x, float cam_z) {
    int cx = (int)floorf(cam_x);
    int cz = (int)floorf(cam_z);

    int ddx = cx - s->last_cx;
    int ddz = cz - s->last_cz;
    if (s->last_cx != 0x7fffffff &&
        ddx * ddx + ddz * ddz < WATER_FOAM_RESCAN_DIST * WATER_FOAM_RESCAN_DIST)
        return 0;   // not far enough, keep the old foam

    s->last_cx = cx;
    s->last_cz = cz;
    darr_clear(s->points);

    // wave energy makes foam more prominent in rough water
    float energy = water_wave_field_energy(f);
    float boost  = 0.5f + 4.0f * energy;
    if (boost > 1.5f) boost = 1.5f;

    int y = s->surface_y;
    int r = s->region;
    // step by 1 block; for big regions this is a lot of lookups but it only
    // runs when you cross a region boundary, not per frame. good enough.
    for (int z = cz - r; z <= cz + r; z++) {
        for (int x = cx - r; x <= cx + r; x++) {
            if (!is_water(w, x, y, z)) continue;
            // must be a *surface* cell — open (air/non-water) directly above
            if (is_water(w, x, y + 1, z)) continue;

            float st = shoreline_strength(w, x, y, z);
            if (st <= 0.0f) continue;

            water_foam_point p;
            p.x        = (float)x + 0.5f;
            p.z        = (float)z + 0.5f;
            p.strength = st * boost;
            if (p.strength > 1.0f) p.strength = 1.0f;
            // hash the coords into a stable phase so neighbouring foam doesnt
            // pulse together
            unsigned h = (unsigned)(x * 73856093) ^ (unsigned)(z * 19349663);
            p.phase = (float)(h & 1023) / 1023.0f * 6.2831853f;
            darr_push(s->points, p);
        }
    }
    return (int)darr_len(s->points);
}

int water_foam_count(const water_foam_set *s) {
    return (int)darr_len(s->points);
}
