#include "villager_poi.h"
#include "../../util/darray.h"
#include "../../math/vec3.h"

#include <stddef.h>

// how long a freshly-released poi stays "warm" before another villager may
// grab it. stops two villagers ping-ponging a single bed every other tick.
#define POI_TICKET_COOLDOWN 3.0f

void villager_poi_init(villager_poi_set *s) {
    s->list = NULL;
}

void villager_poi_free(villager_poi_set *s) {
    darr_free(s->list);
}

static int find_at(villager_poi_set *s, villager_poi_kind kind,
                   int wx, int wy, int wz) {
    for (size_t i = 0; i < darr_len(s->list); i++) {
        villager_poi *p = &s->list[i];
        if (p->valid && p->kind == kind &&
            p->wx == wx && p->wy == wy && p->wz == wz)
            return (int)i;
    }
    return -1;
}

int villager_poi_add(villager_poi_set *s, villager_poi_kind kind,
                     int wx, int wy, int wz) {
    int existing = find_at(s, kind, wx, wy, wz);
    if (existing >= 0) return existing;

    // reuse a dead slot if we have one, keeps the array from creeping.
    for (size_t i = 0; i < darr_len(s->list); i++) {
        if (!s->list[i].valid) {
            villager_poi *p = &s->list[i];
            p->wx = wx; p->wy = wy; p->wz = wz;
            p->kind = (uint8_t)kind;
            p->valid = 1;
            p->owner = 0;
            p->ticket = 0.0f;
            return (int)i;
        }
    }

    villager_poi p = { wx, wy, wz, (uint8_t)kind, 1, 0, 0.0f };
    darr_push(s->list, p);
    return (int)(darr_len(s->list) - 1);
}

void villager_poi_remove(villager_poi_set *s, int wx, int wy, int wz) {
    for (size_t i = 0; i < darr_len(s->list); i++) {
        villager_poi *p = &s->list[i];
        if (p->valid && p->wx == wx && p->wy == wy && p->wz == wz) {
            p->valid = 0;
            p->owner = 0;
            p->ticket = 0.0f;
            // don't break: a bed and a bell could theoretically share a
            // column in weird builds. clear them all.
        }
    }
}

int villager_poi_nearest(villager_poi_set *s, villager_poi_kind kind,
                         vec3 from, uint32_t who, float range_sq) {
    int    best = -1;
    float  best_d = 0.0f;
    for (size_t i = 0; i < darr_len(s->list); i++) {
        villager_poi *p = &s->list[i];
        if (!p->valid || p->kind != kind) continue;
        if (p->owner != 0 && p->owner != who) continue;
        if (p->ticket > 0.0f && p->owner != who) continue;

        vec3  pp = { (float)p->wx + 0.5f, (float)p->wy, (float)p->wz + 0.5f };
        float d  = vec3_distance(from, pp);
        d = d * d;
        if (range_sq > 0.0f && d > range_sq) continue;
        if (best < 0 || d < best_d) { best = (int)i; best_d = d; }
    }
    return best;
}

int villager_poi_claim(villager_poi_set *s, int index, uint32_t who) {
    if (index < 0 || (size_t)index >= darr_len(s->list)) return 0;
    villager_poi *p = &s->list[index];
    if (!p->valid) return 0;
    if (p->owner != 0 && p->owner != who) return 0;
    p->owner = who;
    return 1;
}

void villager_poi_release(villager_poi_set *s, int index, uint32_t who) {
    if (index < 0 || (size_t)index >= darr_len(s->list)) return;
    villager_poi *p = &s->list[index];
    if (p->owner == who) {
        p->owner = 0;
        p->ticket = POI_TICKET_COOLDOWN;
    }
}

void villager_poi_release_all(villager_poi_set *s, uint32_t who) {
    for (size_t i = 0; i < darr_len(s->list); i++) {
        if (s->list[i].owner == who) {
            s->list[i].owner = 0;
            s->list[i].ticket = POI_TICKET_COOLDOWN;
        }
    }
}

void villager_poi_tick(villager_poi_set *s, float dt) {
    for (size_t i = 0; i < darr_len(s->list); i++) {
        if (s->list[i].ticket > 0.0f) {
            s->list[i].ticket -= dt;
            if (s->list[i].ticket < 0.0f) s->list[i].ticket = 0.0f;
        }
    }
}

vec3 villager_poi_pos(const villager_poi_set *s, int index) {
    if (index < 0 || (size_t)index >= darr_len(s->list))
        return vec3_new(0, 0, 0);
    const villager_poi *p = &s->list[index];
    return vec3_new((float)p->wx + 0.5f, (float)p->wy, (float)p->wz + 0.5f);
}
