#include "cavegen_worm.h"

#include "cavegen_noise.h"
#include "../../config.h"
#include "../../math/vec3.h"
#include <math.h>

// private rng for a worm, so all the wobble draws come off one stream and dont
// poke the world rng. same little xorshift the rest of the engine likes.
static uint32_t wrng(uint32_t *s) {
    uint32_t x = *s ? *s : 0x2545f491u;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    *s = x;
    return x;
}
static float wrandf(uint32_t *s) {
    return (float)(wrng(s) >> 8) / (float)(1u << 24);
}

cavegen_worm cavegen_worm_init(vec3 pos, uint32_t stream, const cavegen_params *p) {
    cavegen_worm w;
    w.pos = pos;
    w.rng = stream ? stream : 1u;
    // random initial heading, biased slightly downward so worms tend to dive.
    float yaw   = wrandf(&w.rng) * 6.2831853f;
    float pitch = (wrandf(&w.rng) - 0.65f) * 1.2f;
    float cp = cosf(pitch);
    w.dir = vec3_new(cosf(yaw) * cp, sinf(pitch), sinf(yaw) * cp);
    w.dir = vec3_normalize(w.dir);
    w.radius = p->worm_radius;
    w.steps_left = p->worm_max_steps;
    return w;
}

// carve a sphere of air centred on a cell-space position.
static void carve_sphere(cavegen_grid *g, vec3 c, float r, int flooded_above) {
    int ir = (int)ceilf(r);
    int cx = (int)floorf(c.x), cy = (int)floorf(c.y), cz = (int)floorf(c.z);
    float r2 = r * r;
    for (int dy = -ir; dy <= ir; dy++) {
        for (int dz = -ir; dz <= ir; dz++) {
            for (int dx = -ir; dx <= ir; dx++) {
                float d2 = (float)(dx*dx + dy*dy + dz*dz);
                if (d2 > r2) continue;
                int x = cx + dx, y = cy + dy, z = cz + dz;
                if (!cavegen_grid_in_bounds(x, y, z)) continue;
                if (cavegen_grid_get(g, x, y, z) == CAVEGEN_SOLID) {
                    // below the water table the new void fills with water.
                    int v = (y < flooded_above) ? CAVEGEN_FLOODED : CAVEGEN_WORM;
                    cavegen_grid_set(g, x, y, z, v);
                }
            }
        }
    }
}

int cavegen_worm_step(cavegen_grid *g, cavegen_worm *w, const cavegen_params *p) {
    if (w->steps_left <= 0) return 0;
    w->steps_left--;

    // steer: sample a noise field at the worm head and rotate the heading
    // toward its gradient a little. this is the "perlin" in perlin worm.
    int wx, wy, wz;
    // worm pos is in cell space; convert to world for stable noise sampling.
    cavegen_cell_to_world(g, (int)w->pos.x, (int)w->pos.y, (int)w->pos.z,
                          &wx, &wy, &wz);
    float fx = (float)wx, fy = (float)wy, fz = (float)wz;
    float s = 0.08f;
    float gx = cavegen_value3(fx*s + 11.0f, fy*s, fz*s, p->seed ^ 0x770001u);
    float gy = cavegen_value3(fx*s, fy*s + 19.0f, fz*s, p->seed ^ 0x770002u);
    float gz = cavegen_value3(fx*s, fy*s, fz*s + 7.0f, p->seed ^ 0x770003u);

    vec3 turn = vec3_new(gx, gy * 0.5f, gz);   // damp vertical so floors stay walkable-ish
    w->dir = vec3_add(w->dir, vec3_scale(turn, p->worm_pitch_max));
    // dont let it climb or dive past the pitch cap
    if (w->dir.y >  0.7f) w->dir.y =  0.7f;
    if (w->dir.y < -0.8f) w->dir.y = -0.8f;
    w->dir = vec3_normalize(w->dir);

    // wobble the radius along the length so tunnels pinch and bulge.
    float rv = cavegen_value3(fx*0.2f, fy*0.2f, fz*0.2f, p->seed ^ 0x4ad10u);
    float r = p->worm_radius + rv * p->worm_radius_var;
    if (r < 1.0f) r = 1.0f;

    carve_sphere(g, w->pos, r, p->water_table - 0 /*flood under table*/);

    // advance
    w->pos = vec3_add(w->pos, vec3_scale(w->dir, p->worm_step));

    // bail if we wandered well outside the padded region (give a little slack so
    // the worm can clip a corner and come back).
    if (w->pos.x < -2 || w->pos.x > CAVEGEN_DIM_X + 2 ||
        w->pos.z < -2 || w->pos.z > CAVEGEN_DIM_Z + 2 ||
        w->pos.y < (float)p->min_y - 2 || w->pos.y > CAVEGEN_DIM_Y + 2)
        return 0;

    return 1;
}

void cavegen_worm_run(cavegen_grid *g, cavegen_worm *w, const cavegen_params *p) {
    while (cavegen_worm_step(g, w, p)) { /* keep wriggling */ }
}

void cavegen_worm_spawn_all(cavegen_grid *g, const cavegen_params *p) {
    // derive worm seeds from chunk coords so worms are stable per chunk and
    // shared across the pad overlap with neighbours.
    uint32_t base = cavegen_hash3(g->origin.chunk_cx, 0, g->origin.chunk_cz,
                                  p->seed ^ 0x90c0u);
    for (int i = 0; i < p->worm_count; i++) {
        uint32_t s = cavegen_hash1(base + (uint32_t)i * 2654435761u, p->seed);
        // spawn point biased to the lower-mid of the region where caves belong.
        float px = (float)(s % CAVEGEN_DIM_X);
        s = cavegen_hash1(s, 0x11u);
        float pz = (float)(s % CAVEGEN_DIM_Z);
        s = cavegen_hash1(s, 0x22u);
        int span = (CAVEGEN_DIM_Y / 2) - p->min_y;
        if (span < 1) span = 1;
        float py = (float)(p->min_y + (int)(s % (uint32_t)span));

        cavegen_worm w = cavegen_worm_init(vec3_new(px, py, pz),
                                           cavegen_hash1(s, 0xbeefu), p);
        cavegen_worm_run(g, &w, p);
    }
}
