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
if (sp->timer < sp->interval) return;
sp->timer = 0;
sp->hostile_count = count_type(mr, 1);
sp->passive_count = count_type(mr, 0);
int is_night = (day_hour < 6.0f || day_hour > 20.0f);
vec3 pos = find_spawn_pos(w, player_pos);
entity_type t = rng_range(&sp_rng, 0, 1) ? ET_COW : ET_PIG;
for (int i = 0;
i < pack;
for (int i = 0;
i < MAX_MOBS;
}
