#include "cavegen_ravine.h"

#include "cavegen_noise.h"
#include "../../config.h"
#include <math.h>

// same little private xorshift as the worms.
static uint32_t rrng(uint32_t *s) {
    uint32_t x = *s ? *s : 0x6d2b79f5u;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    *s = x;
    return x;
}
static float rrandf(uint32_t *s) {
    return (float)(rrng(s) >> 8) / (float)(1u << 24);
}

cavegen_ravine cavegen_ravine_init(vec3 pos, uint32_t stream) {
    cavegen_ravine r;
    r.pos = pos;
    r.rng = stream ? stream : 1u;
    r.yaw = rrandf(&r.rng) * 6.2831853f;
    r.width  = 1.6f + rrandf(&r.rng) * 1.8f;     // 1.6 .. 3.4 wide
    r.height = 10.0f + rrandf(&r.rng) * 14.0f;   // 10 .. 24 tall, the drama
    r.steps_left = 60 + (int)(rrandf(&r.rng) * 50.0f);
    return r;
}

// elliptical brush: wide-ish in xz, very tall in y. carved straight down from
// the walker head so the ravine has a real floor and isnt just a floating gash.
static void carve_blade(cavegen_grid *g, vec3 c, float w, float h,
                        const cavegen_params *p) {
    int iw = (int)ceilf(w);
    int ih = (int)ceilf(h);
    int cx = (int)floorf(c.x), cy = (int)floorf(c.y), cz = (int)floorf(c.z);

    for (int dy = -ih; dy <= ih; dy++) {
        for (int dz = -iw; dz <= iw; dz++) {
            for (int dx = -iw; dx <= iw; dx++) {
                // normalised ellipsoid test, vertical axis stretched.
                float fx = (float)dx / w;
                float fz = (float)dz / w;
                float fy = (float)dy / h;
                if (fx*fx + fz*fz + fy*fy > 1.0f) continue;

                int x = cx + dx, y = cy + dy, z = cz + dz;
                if (!cavegen_grid_in_bounds(x, y, z)) continue;
                if (y < p->min_y) continue;
                if (cavegen_grid_get(g, x, y, z) != CAVEGEN_SOLID) continue;

                int v = (y < p->water_table) ? CAVEGEN_FLOODED : CAVEGEN_AIR;
                cavegen_grid_set(g, x, y, z, v);
            }
        }
    }
}

int cavegen_ravine_step(cavegen_grid *g, cavegen_ravine *r, const cavegen_params *p) {
    if (r->steps_left <= 0) return 0;
    r->steps_left--;

    carve_blade(g, r->pos, r->width, r->height, p);

    // gentle meander in yaw only — ravines wander but dont coil.
    int wx, wy, wz;
    cavegen_cell_to_world(g, (int)r->pos.x, (int)r->pos.y, (int)r->pos.z,
                          &wx, &wy, &wz);
    float n = cavegen_value3((float)wx * 0.05f, 0.0f, (float)wz * 0.05f,
                             p->seed ^ 0x4a71u);
    r->yaw += n * 0.18f;

    // width tapers toward the ends so it closes off instead of stopping flat.
    float t = (float)r->steps_left;
    if (t < 8.0f) r->width *= 0.9f;

    r->pos.x += cosf(r->yaw);
    r->pos.z += sinf(r->yaw);
    // very slow downward drift so the floor descends along its length.
    r->pos.y -= 0.05f;

    if (r->pos.x < -2 || r->pos.x > CAVEGEN_DIM_X + 2 ||
        r->pos.z < -2 || r->pos.z > CAVEGEN_DIM_Z + 2 ||
        r->width < 1.0f)
        return 0;
    return 1;
}

int cavegen_ravine_maybe_spawn(cavegen_grid *g, const cavegen_params *p) {
    // ~1 in 14 chunks. hashed off chunk coords so its deterministic.
    uint32_t h = cavegen_hash3(g->origin.chunk_cx, 99, g->origin.chunk_cz,
                               p->seed ^ 0x9a71f1u);
    if ((h % 14u) != 0u) return 0;

    // start somewhere in the lower-middle of the region.
    uint32_t s = cavegen_hash1(h, 0x5151u);
    float px = (float)(CAVEGEN_PAD + (s % (uint32_t)CHUNK_SIZE_X));
    s = cavegen_hash1(s, 0x7u);
    float pz = (float)(CAVEGEN_PAD + (s % (uint32_t)CHUNK_SIZE_Z));
    s = cavegen_hash1(s, 0x9u);
    // put the head high enough that the tall blade reaches down toward the floor.
    int top = (CAVEGEN_DIM_Y * 3) / 5;
    int lo  = p->min_y + 14;
    if (top <= lo) top = lo + 1;
    float py = (float)(lo + (int)(s % (uint32_t)(top - lo)));

    cavegen_ravine r = cavegen_ravine_init(vec3_new(px, py, pz),
                                           cavegen_hash1(s, 0xca7u));
    while (cavegen_ravine_step(g, &r, p)) { /* slash */ }
    return 1;
}
