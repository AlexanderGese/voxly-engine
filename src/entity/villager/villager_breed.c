#include "villager_breed.h"
#include "../../util/darray.h"

#include <math.h>
#include <stddef.h>

// idle food drain per second. a villager who never works or trades slowly
// loses willingness. small, so it takes a while to matter.
#define BREED_FOOD_DRAIN 0.05f
#define BREED_FOOD_MAX   20.0f

int villager_breed_is_willing(const villager *v, float food, float cooldown) {
    if (v->is_baby) return 0;
    if (v->prof == VILLAGER_PROF_NITWIT) return 0;   // nitwits abstain
    if (cooldown > 0.0f) return 0;
    if (v->hp < v->max_hp / 2) return 0;             // too hurt to bother
    return food >= (float)VILLAGER_BREED_FOOD_NEED;
}

float villager_breed_feed(float food, float amount) {
    food += amount;
    if (food > BREED_FOOD_MAX) food = BREED_FOOD_MAX;
    if (food < 0.0f) food = 0.0f;
    return food;
}

// is there a bed nobody has claimed yet? babies need somewhere to live.
static int has_free_bed(villager_manager *m) {
    // reuse the poi registry: a bell-anchored search from the first villager.
    if (villager_manager_count(m) == 0) return 0;
    villager *first = NULL;
    // grab any villager to use as a search origin.
    for (size_t i = 0; i < (size_t)villager_manager_count(m); i++) {
        villager *v = &m->vills[i];
        if (villager_is_alive(v)) { first = v; break; }
    }
    if (!first) return 0;
    int idx = villager_poi_nearest(&m->pois, VILLAGER_POI_BED, first->pos, 0, 0.0f);
    return idx >= 0;
}

int villager_breed_step(villager_manager *m, float *food, float *cooldown, float dt) {
    int count = villager_manager_count(m);
    if (count < 2) {
        for (int i = 0; i < count; i++) {
            if (cooldown[i] > 0.0f) cooldown[i] -= dt;
            food[i] = villager_breed_feed(food[i], -BREED_FOOD_DRAIN * dt);
        }
        return 0;
    }

    // top-up + drain pass. working/gathering villagers nibble food back up,
    // panicking ones lose appetite.
    for (int i = 0; i < count; i++) {
        villager *v = &m->vills[i];
        if (cooldown[i] > 0.0f) cooldown[i] -= dt;

        float delta = -BREED_FOOD_DRAIN * dt;
        if (v->activity == VILLAGER_ACT_WORK)   delta += 0.20f * dt;
        if (v->activity == VILLAGER_ACT_GATHER) delta += 0.08f * dt;
        if (v->activity == VILLAGER_ACT_PANIC)  delta -= 0.10f * dt;
        food[i] = villager_breed_feed(food[i], delta);
    }

    int babies = 0;
    if (!has_free_bed(m)) return 0;   // no room at the inn

    // O(n^2) pair scan. villages are small; this is fine. once a pair breeds
    // both go on cooldown and we move on.
    for (int i = 0; i < count; i++) {
        villager *a = &m->vills[i];
        if (!villager_breed_is_willing(a, food[i], cooldown[i])) continue;

        for (int j = i + 1; j < count; j++) {
            villager *b = &m->vills[j];
            if (!villager_breed_is_willing(b, food[j], cooldown[j])) continue;
            if (vec3_distance(a->pos, b->pos) > VILLAGER_BREED_RANGE) continue;

            // spawn the baby at the midpoint. note: this appends to the
            // manager arrays, which may realloc — so we must NOT keep using
            // a/b after this. break out of the inner loop immediately.
            vec3 mid = vec3_scale(vec3_add(a->pos, b->pos), 0.5f);
            villager_manager_spawn_baby(m, mid);

            cooldown[i] = VILLAGER_BREED_COOLDOWN;
            cooldown[j] = VILLAGER_BREED_COOLDOWN;
            food[i] = villager_breed_feed(food[i], -(float)VILLAGER_BREED_FOOD_NEED);
            food[j] = villager_breed_feed(food[j], -(float)VILLAGER_BREED_FOOD_NEED);
            babies++;
            break;   // a is spent; pointers possibly stale after spawn
        }
    }
    return babies;
}
