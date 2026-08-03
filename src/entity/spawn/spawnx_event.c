#include "spawnx_event.h"
#include "spawnx_commit.h"
#include "mspawn_query.h"
#include <math.h>

void spawnx_event_begin(spawnx_event *ev, vec3 focus, biome_id biome,
                        int waves, int wave_budget, float wave_gap,
                        uint32_t id, unsigned seed) {
    ev->phase       = SPAWNX_EV_RUNNING;
    ev->focus       = focus;
    ev->biome       = biome;
    ev->waves_total = waves < 1 ? 1 : waves;
    ev->waves_done  = 0;
    ev->wave_budget = wave_budget < 1 ? 1 : wave_budget;
    ev->wave_gap    = wave_gap;
    ev->wave_timer  = 0.0f;       // first wave fires on the next tick
    ev->ring_min    = 12.0f;
    ev->ring_max    = 28.0f;
    ev->spawned     = 0;
    ev->id          = id;
    mspawn_rng_seed(&ev->rng, mspawn_seed_mix((uint32_t)seed, id));
}

// roll a point on the placement ring around the focus, snapped to the surface.
// returns 1 + fills *out, or 0 if the column has no legal floor right now.
static int ring_point(spawnx_event *ev, world *w, vec3 *out) {
    float ang = mspawn_rng_frange(&ev->rng, 0.0f, 6.2831853f);
    float rad = mspawn_rng_frange(&ev->rng, ev->ring_min, ev->ring_max);
    int wx = (int)floorf(ev->focus.x + cosf(ang) * rad);
    int wz = (int)floorf(ev->focus.z + sinf(ang) * rad);

    int surf = mspawn_query_surface(w, wx, wz);
    int wy = spawnx_settle_y(w, wx, surf, wz, 1);
    if (wy < 0) return 0;

    *out = vec3_new(wx + 0.5f, (float)wy, wz + 0.5f);
    return 1;
}

// fire one wave: draw from the biome roster until the wave budget is spent or we
// stop finding ground, placing each on the ring. returns mobs placed.
static int fire_wave(spawnx_event *ev, world *w, mob_registry *mr,
                     spawnx_region_map *rm) {
    spawnx_roster roster = spawnx_weight_biome(ev->biome);
    roster.budget = ev->wave_budget;

    int placed = 0;
    int guard = 0;                 // bound the loop; ring points can keep failing
    while (roster.budget > 0 && guard < 64) {
        guard++;

        vec3 spot;
        if (!ring_point(ev, w, &spot)) continue;
        if (!spawnx_region_has_room(rm, spot)) continue;

        entity_type type;
        if (!spawnx_weight_draw(&roster, &ev->rng, (int)spot.y, &type)) break;

        spawnx_request req;
        req.pos     = spot;
        req.type    = type;
        req.source  = SPAWNX_SRC_EVENT;
        req.tag     = ev->id;
        req.persist = 0;

        spawnx_result res;
        if (spawnx_commit(w, mr, &req, &res)) {
            spawnx_region_account(rm, res.pos);
            placed++;
        }
        // a failed commit still spent the draw's budget on purpose: a wave that
        // cant find room thins out rather than grinding forever.
    }
    return placed;
}

int spawnx_event_tick(spawnx_event *ev, world *w, mob_registry *mr,
                      spawnx_region_map *rm, float dt) {
    if (ev->phase != SPAWNX_EV_RUNNING) return 0;

    ev->wave_timer -= dt;
    if (ev->wave_timer > 0.0f) return 0;

    int n = fire_wave(ev, w, mr, rm);
    ev->spawned += n;
    ev->waves_done++;

    if (ev->waves_done >= ev->waves_total) {
        ev->phase = SPAWNX_EV_DONE;
    } else {
        ev->wave_timer = ev->wave_gap;
    }
    return n;
}

int spawnx_event_finished(const spawnx_event *ev) {
    return ev->phase == SPAWNX_EV_DONE || ev->phase == SPAWNX_EV_NONE;
}
