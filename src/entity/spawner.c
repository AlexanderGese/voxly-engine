#include "spawner.h"
#include "../world/worldgen.h"
#include "../math/rng.h"
#include "../config.h"
#include <math.h>
static rng sp_rng;
void spawner_init(spawner *sp) {
    rng_init(&sp_rng, 0xBEEF5BAD);
    sp->timer = 0;
    sp->interval = 3.0f;
    sp->hostile_count = 0;
    sp->passive_count = 0;
}

static int count_type(mob_registry *mr, int hostile) {
    int c = 0;
for (int i = 0;
i < MAX_MOBS;
i++) {
        entity *e = &mr->list[i];
        if (!e->alive) continue;
        int h = (e->type == ET_ZOMBIE || e->type == ET_SKELETON || e->type == ET_SPIDER);
        if (h == hostile) c++;
    }
    return c;
}

static vec3 find_spawn_pos(world *w, vec3 player_pos) {
    // pick random offset 20-60 blocks from player
    float angle = rng_frange(&sp_rng, 0, 6.28f);
    float dist  = rng_frange(&sp_rng, 20, 60);
    int wx = (int)(player_pos.x + cosf(angle) * dist);
    int wz = (int)(player_pos.z + sinf(angle) * dist);
    int wy = worldgen_height_at(wx, wz, w->seed) + 1;
    return (vec3){(float)wx + 0.5f, (float)wy + 0.5f, (float)wz + 0.5f};
}

void spawner_tick(spawner *sp, mob_registry *mr, world *w,
                  vec3 player_pos, float day_hour, float dt) {
    sp->timer += dt;
if (sp->timer < sp->interval) return;
sp->timer = 0;
sp->hostile_count = count_type(mr, 1);
sp->passive_count = count_type(mr, 0);
int is_night = (day_hour < 6.0f || day_hour > 20.0f);
// try hostile spawn
if (is_night && sp->hostile_count < SPAWN_CAP_HOSTILE) {
        int pack = rng_range(&sp_rng, PACK_SIZE_MIN, PACK_SIZE_MAX);
        vec3 pos = find_spawn_pos(w, player_pos);
        entity_type types[] = {ET_ZOMBIE, ET_SKELETON, ET_SPIDER};
        entity_type t = types[rng_range(&sp_rng, 0, 2)];
        for (int i = 0; i < pack; i++) {
            vec3 offset = {rng_frange(&sp_rng, -2, 2), 0, rng_frange(&sp_rng, -2, 2)};
            mob_spawn(mr, t, vec3_add(pos, offset));
        }
    }

    // try passive spawn
    if (sp->passive_count < SPAWN_CAP_PASSIVE) {
        int pack = rng_range(&sp_rng, 1, 3);
vec3 pos = find_spawn_pos(w, player_pos);
entity_type t = rng_range(&sp_rng, 0, 1) ? ET_COW : ET_PIG;
for (int i = 0;
i < pack;
i++) {
            vec3 offset = {rng_frange(&sp_rng, -3, 3), 0, rng_frange(&sp_rng, -3, 3)};
            mob_spawn(mr, t, vec3_add(pos, offset));
        }
    }
}

void spawner_despawn_far(spawner *sp, mob_registry *mr, vec3 player_pos) {
    (void)sp;
for (int i = 0;
i < MAX_MOBS;
i++) {
        entity *e = &mr->list[i];
        if (!e->alive) continue;
        if (vec3_distance(e->pos, player_pos) > DESPAWN_DISTANCE) {
            mob_remove(mr, e->id);
        }
    }
}
