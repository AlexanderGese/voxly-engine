#include "villager_manager.h"
#include "villager_brain.h"
#include "villager_workstation.h"
#include "../../util/darray.h"
#include "../../math/rng.h"

#include <math.h>
#include <stddef.h>

// how often we rescan the world for new workstations/beds/bells, and how big
// the scan box is. the scan is the priciest thing here so we keep it lazy.
#define MGR_SCAN_INTERVAL 6.0f
#define MGR_SCAN_RADIUS   14

// radius within which a witnessed attack spreads negative gossip.
#define MGR_RUMOR_RADIUS  10.0f

void villager_manager_init(villager_manager *m) {
    m->vills = NULL;
    m->navs = NULL;
    villager_poi_init(&m->pois);
    m->next_id = 1;          // 0 is the "no villager" sentinel
    m->scan_timer = 0.0f;
    m->poi_origin_x = 0;
    m->poi_origin_z = 0;
}

void villager_manager_free(villager_manager *m) {
    darr_free(m->vills);
    darr_free(m->navs);
    villager_poi_free(&m->pois);
}

static uint32_t add_villager(villager_manager *m, villager v) {
    villager_nav nav;
    villager_nav_reset(&nav);
    darr_push(m->vills, v);
    darr_push(m->navs, nav);
    return v.id;
}

uint32_t villager_manager_spawn(villager_manager *m, villager_profession prof, vec3 pos) {
    uint32_t id = m->next_id++;
    villager v = villager_make(id, prof, pos, (uint64_t)id * 0x9E3779B97F4A7C15ull);
    return add_villager(m, v);
}

uint32_t villager_manager_spawn_baby(villager_manager *m, vec3 pos) {
    uint32_t id = m->next_id++;
    villager v = villager_make_baby(id, pos, (uint64_t)id * 0x9E3779B97F4A7C15ull);
    return add_villager(m, v);
}

villager *villager_manager_get(villager_manager *m, uint32_t id) {
    if (id == 0) return NULL;
    for (size_t i = 0; i < darr_len(m->vills); i++)
        if (m->vills[i].id == id) return &m->vills[i];
    return NULL;
}

// swap-remove a dead villager, releasing its POIs first. keeps vills/navs in
// lockstep.
static void remove_at(villager_manager *m, size_t i) {
    villager_poi_release_all(&m->pois, m->vills[i].id);
    size_t last = darr_len(m->vills) - 1;
    if (i != last) {
        m->vills[i] = m->vills[last];
        m->navs[i]  = m->navs[last];
    }
    darr_hdr(m->vills)->len--;
    darr_hdr(m->navs)->len--;
}

void villager_manager_tick(villager_manager *m, world *w, float day_t,
                           vec3 threat, int threat_active, float dt) {
    villager_poi_tick(&m->pois, dt);

    // lazy world rescan, centered on the first (representative) villager so
    // the poi set tracks where the villagers actually are.
    m->scan_timer -= dt;
    if (m->scan_timer <= 0.0f && darr_len(m->vills) > 0) {
        m->scan_timer = MGR_SCAN_INTERVAL;
        villager_workstation_scan(&m->pois, w, m->vills[0].pos, MGR_SCAN_RADIUS);
        m->poi_origin_x = (int)floorf(m->vills[0].pos.x);
        m->poi_origin_z = (int)floorf(m->vills[0].pos.z);
    }

    villager_brain_ctx ctx;
    ctx.day_t = day_t;
    ctx.threat_pos = threat;
    ctx.threat_active = threat_active;
    // village center for GATHER: nearest bell to villager[0], if any.
    int bell = villager_poi_nearest(&m->pois, VILLAGER_POI_BELL,
                                    darr_len(m->vills) ? m->vills[0].pos : threat,
                                    0, 0.0f);
    if (bell >= 0) { ctx.bell_pos = villager_poi_pos(&m->pois, bell); ctx.have_bell = 1; }
    else           { ctx.bell_pos = threat; ctx.have_bell = 0; }

    for (size_t i = 0; i < darr_len(m->vills); ) {
        villager *v = &m->vills[i];
        if (!villager_is_alive(v)) { remove_at(m, i); continue; }
        villager_brain_tick(v, &m->navs[i], &m->pois, w, &ctx, dt);
        i++;
    }
}

int villager_manager_hurt(villager_manager *m, uint32_t id, int amount, vec3 attacker) {
    villager *v = villager_manager_get(m, id);
    if (!v) return 0;

    int died = villager_hurt(v, amount);
    vec3 at = v->pos;

    // every villager who could plausibly witness it sours on the attacker.
    for (size_t i = 0; i < darr_len(m->vills); i++) {
        villager *o = &m->vills[i];
        if (vec3_distance(o->pos, at) <= MGR_RUMOR_RADIUS) {
            villager_gossip_add(&o->gossip,
                died ? VILLAGER_GOSSIP_MAJOR_NEGATIVE
                     : VILLAGER_GOSSIP_MINOR_NEGATIVE,
                died ? 8 : 3);
            // make them jumpy too: point them away from the attacker.
            o->yaw = atan2f(o->pos.x - attacker.x, o->pos.z - attacker.z);
        }
    }
    return died;
}

int villager_manager_trade(villager_manager *m, uint32_t id, int slot,
                           int player_has, block_id *out_item,
                           int *out_count, int *paid) {
    villager *v = villager_manager_get(m, id);
    if (!v) return 0;
    if (!villager_trade_can(&v->trades, slot, player_has)) return 0;

    // price-gate: make sure the player can actually pay the gossip-adjusted
    // price, not just the base want_count.
    int price = villager_trade_price(&v->trades, slot, &v->gossip);
    if (player_has < price) return 0;

    if (!villager_trade_do(&v->trades, slot, &v->gossip, out_item, out_count, paid))
        return 0;

    v->level = v->trades.level;
    // trading is good for the soul; small goodwill bump beyond the in-do one.
    villager_gossip_add(&v->gossip, VILLAGER_GOSSIP_MINOR_POSITIVE, 1);
    return 1;
}

int villager_manager_count(const villager_manager *m) {
    return (int)darr_len(m->vills);
}
