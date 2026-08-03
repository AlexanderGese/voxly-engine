#include "spawnx_blockspawner.h"
#include "spawnx_commit.h"
#include <math.h>
static vec3 block_center(const spawnx_blockspawner *bs) {
    return vec3_new(bs->wx + 0.5f, (float)bs->wy, bs->wz + 0.5f);
}

void spawnx_bs_make(spawnx_blockspawner *bs, int wx, int wy, int wz,
                    entity_type type, biome_id biome, unsigned world_seed) {
    bs->wx = wx;
bs->wy = wy;
bs->wz = wz;
bs->type = type;
bs->biome = biome;
bs->state = SPAWNX_BS_ST_IDLE;
bs->delay = 0.0f;
bs->active = 1;
bs->last_burst = 0;
uint32_t s = mspawn_hash3(wx, wy, wz, (uint32_t)world_seed);
mspawn_rng_seed(&bs->rng, s);
}

// horizontal distance from the cage to a point. cheap gate before we do work.
static float dist_xz(const spawnx_blockspawner *bs, vec3 p) {
    float dx = p.x - (bs->wx + 0.5f);
    float dz = p.z - (bs->wz + 0.5f);
    return sqrtf(dx * dx + dz * dz);
}

// count this spawner's mob kind within SPAWNX_BS_NEARBY of the cage. the nearby
// cap reads this so an unattended spawner stops once its little crowd exists.
static int count_nearby(const spawnx_blockspawner *bs, const mob_registry *mr) {
    float r2 = SPAWNX_BS_NEARBY * SPAWNX_BS_NEARBY;
vec3 c = block_center(bs);
int n = 0;
for (int i = 0;
i < mr->count;
i++) {
        const entity *e = &mr->list[i];
        if (!e->alive || e->type != bs->type) continue;
        float dx = e->pos.x - c.x, dy = e->pos.y - c.y, dz = e->pos.z - c.z;
        if (dx * dx + dy * dy + dz * dz < r2) n++;
    }
    return n;
}

// fire one burst: scatter SPAWNX_BS_BURST attempts in a box around the cage and
// commit each. returns the number that actually landed.
static int do_burst(spawnx_blockspawner *bs, world *w, mob_registry *mr,
                    spawnx_region_map *rm) {
    int placed = 0;
    for (int a = 0; a < SPAWNX_BS_ST_BURST; a++) {
        int ox = mspawn_rng_range(&bs->rng, -SPAWNX_BS_SPREAD, SPAWNX_BS_SPREAD);
        int oz = mspawn_rng_range(&bs->rng, -SPAWNX_BS_SPREAD, SPAWNX_BS_SPREAD);
        int oy = mspawn_rng_range(&bs->rng, -1, 1);

        vec3 want = vec3_new(bs->wx + ox + 0.5f, (float)(bs->wy + oy),
                             bs->wz + oz + 0.5f);

        if (!spawnx_region_has_room(rm, want)) continue;

        spawnx_request req;
        req.pos     = want;
        req.type    = bs->type;
        req.source  = SPAWNX_SRC_BLOCK;
        req.tag     = mspawn_hash3(bs->wx, bs->wy, bs->wz, 0);
        req.persist = 0;

        spawnx_result res;
        if (spawnx_commit(w, mr, &req, &res)) {
            spawnx_region_account(rm, res.pos);
            placed++;
        }
    }
    return placed;
}

int spawnx_bs_tick(spawnx_blockspawner *bs, world *w, mob_registry *mr,
                   spawnx_region_map *rm, vec3 player_pos, float dt) {
    if (!bs->active) return 0;
bs->last_burst = 0;
int in_range = dist_xz(bs, player_pos) <= SPAWNX_BS_RANGE;
}
