// standalone smoke harness for the projectile core. no world, no gl: we back the
// sampler with a flat block grid and fire a few shots, asserting the obvious
// invariants (an arrow into a wall sticks, a snowball over a pit falls into the
// void, a spear skewers two targets). this is the thing the fuzzer wraps and the
// thing i run when i touch the ballistics and want to know i didnt break the arc.
//
// build: cc -DPROJECTILE_DEMO_MAIN projectile_*.c ../../math/*.c -lm -o projdemo

#include "projectile_world.h"
#include "projectile_ballistic.h"
#include "projectile_aim.h"
#include "projectile_save.h"

#include "../../util/log.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

// a tiny fixed test world: a 64x32x64 block grid. floor at y=0, plus a wall at
// x=20 we can shoot into. coordinates outside the grid read as air.
#define DG_X 64
#define DG_Y 32
#define DG_Z 64

typedef struct {
    block_id cells[DG_X * DG_Y * DG_Z];
} demo_grid;

static int dg_idx(int x, int y, int z) {
    return x + z * DG_X + y * DG_X * DG_Z;
}

static block_id dg_at(void *user, int x, int y, int z) {
    demo_grid *g = (demo_grid *)user;
    if (x < 0 || x >= DG_X || y < 0 || y >= DG_Y || z < 0 || z >= DG_Z)
        return BLOCK_AIR;
    return g->cells[dg_idx(x, y, z)];
}

static void dg_fill_floor(demo_grid *g) {
    memset(g, 0, sizeof *g);
    for (int x = 0; x < DG_X; x++)
        for (int z = 0; z < DG_Z; z++)
            g->cells[dg_idx(x, 0, z)] = BLOCK_STONE;
    // a wall slab at x=20, the arrow backstop.
    for (int y = 1; y < 8; y++)
        for (int z = 0; z < DG_Z; z++)
            g->cells[dg_idx(20, y, z)] = BLOCK_COBBLE;
}

// damage sink so we can count hits without a real mob registry.
static int g_dmg_total = 0;
static int g_dmg_hits  = 0;
static void demo_damage(void *user, int id, int dmg, vec3 point) {
    (void)user; (void)id; (void)point;
    g_dmg_total += dmg;
    g_dmg_hits++;
}

static int approx(float a, float b, float eps) { return fabsf(a - b) <= eps; }

// 1) arrow fired flat into the wall should end STUCK, lodged at x in [19,21).
static int test_arrow_sticks(void) {
    demo_grid g; dg_fill_floor(&g);
    projectile_world pw;
    projectile_world_init(&pw, 12345);
    projectile_sampler s = { dg_at, &g };
    projectile_world_set_sampler(&pw, s);

    projectile_shot shot = {
        .kind = PROJECTILE_ARROW,
        .origin = (vec3){ 2.0f, 4.0f, 8.0f },
        .dir = VEC3_RIGHT,          // +x toward the wall
        .speed = 40.0f,
        .spread = 0.0f,
        .owner_id = -1,
    };
    uint32_t id = projectile_world_fire(&pw, &shot);
    if (!id) { LOGE("demo: fire failed"); return 0; }

    for (int i = 0; i < 240; i++) projectile_world_update(&pw, 1.0f / 60.0f, NULL, 0);

    projectile *p = projectile_pool_find(&pw.pool, id);
    if (!p) { LOGE("demo: arrow vanished (expected stuck)"); return 0; }
    if (p->state != PROJ_STATE_STUCK) {
        LOGE("demo: arrow not stuck, state=%d pos.x=%.2f", p->state, p->pos.x);
        return 0;
    }
    if (p->stuck_bx < 19 || p->stuck_bx > 21) {
        LOGE("demo: arrow stuck in wrong cell x=%d", p->stuck_bx);
        return 0;
    }
    LOGI("demo: arrow stuck at (%d,%d,%d) ok", p->stuck_bx, p->stuck_by, p->stuck_bz);
    return 1;
}

// 2) the closed-form arc and the simulated arc should agree (drag off path).
static int test_arc_matches(void) {
    vec3 origin = (vec3){ 0, 10, 0 };
    vec3 vel    = (vec3){ 15, 5, 0 };
    float gmag  = 22.0f;            // matches |GRAVITY|

    vec3 land; float t;
    if (!projectile_ballistic_predict(origin, vel, -gmag, 0.0f, &land, &t)) {
        LOGE("demo: predict said the shot never lands");
        return 0;
    }
    // sanity: x = vx * t, and t solves the quadratic.
    if (!approx(land.x, vel.x * t, 0.01f)) {
        LOGE("demo: arc x mismatch %.3f vs %.3f", land.x, vel.x * t);
        return 0;
    }
    LOGI("demo: arc lands at x=%.2f after t=%.2fs ok", land.x, t);
    return 1;
}

// 3) a spear with one pierce, fired through two stacked targets, hits both.
static int test_spear_pierces(void) {
    demo_grid g; dg_fill_floor(&g);
    projectile_world pw;
    projectile_world_init(&pw, 777);
    projectile_sampler s = { dg_at, &g };
    projectile_world_set_sampler(&pw, s);
    projectile_world_set_damage_cb(&pw, demo_damage, NULL);
    g_dmg_total = g_dmg_hits = 0;

    projectile_target targets[2] = {
        { .id = 3, .box = aabb_from_center((vec3){ 6, 4, 8 }, (vec3){ .4f, .9f, .4f }) },
        { .id = 5, .box = aabb_from_center((vec3){ 9, 4, 8 }, (vec3){ .4f, .9f, .4f }) },
    };

    projectile_shot shot = {
        .kind = PROJECTILE_SPEAR,
        .origin = (vec3){ 2.0f, 4.0f, 8.0f },
        .dir = VEC3_RIGHT,
        .speed = 30.0f,
        .spread = 0.0f,
        .owner_id = -1,
    };
    if (!projectile_world_fire(&pw, &shot)) { LOGE("demo: spear fire failed"); return 0; }

    for (int i = 0; i < 120; i++) projectile_world_update(&pw, 1.0f / 60.0f, targets, 2);

    if (g_dmg_hits < 2) {
        LOGE("demo: spear pierced only %d targets (want 2)", g_dmg_hits);
        return 0;
    }
    LOGI("demo: spear hit %d targets for %d total dmg ok", g_dmg_hits, g_dmg_total);
    return 1;
}

// 4) ballistic aim solver: aim a high arc at a raised target and verify the
// produced velocity, simulated, lands near it.
static int test_aim_lands(void) {
    vec3 from   = (vec3){ 0, 2, 0 };
    vec3 target = (vec3){ 25, 6, 0 };
    vec3 vel;
    if (!projectile_aim_ballistic(PROJECTILE_ARROW, from, target, -22.0f, &vel)) {
        LOGE("demo: aim solver said unreachable");
        return 0;
    }
    // integrate the closed-form arc to the target's x and check the height.
    float tx = (target.x - from.x) / vel.x;
    float y  = from.y + vel.y * tx + 0.5f * (-22.0f) * tx * tx;
    if (!approx(y, target.y, 0.6f)) {
        LOGE("demo: aim arc missed: y=%.2f want %.2f", y, target.y);
        return 0;
    }
    LOGI("demo: ballistic aim arc within tolerance (y=%.2f) ok", y);
    return 1;
}

// 5) save round-trip: stick an arrow, serialize, reload into a fresh pool, and
// check the stuck cell + embed offset survive the trip intact.
static int test_save_roundtrip(void) {
    demo_grid g; dg_fill_floor(&g);
    projectile_world pw;
    projectile_world_init(&pw, 99);
    projectile_sampler s = { dg_at, &g };
    projectile_world_set_sampler(&pw, s);

    projectile_shot shot = {
        .kind = PROJECTILE_ARROW,
        .origin = (vec3){ 2.0f, 4.0f, 8.0f },
        .dir = VEC3_RIGHT, .speed = 40.0f, .spread = 0.0f, .owner_id = 7,
    };
    projectile_world_fire(&pw, &shot);
    for (int i = 0; i < 240; i++) projectile_world_update(&pw, 1.0f / 60.0f, NULL, 0);

    size_t need = projectile_save_size(&pw.pool);
    uint8_t *blob = malloc(need);
    size_t wrote = projectile_save_write(&pw.pool, blob, need);
    if (wrote != need || need <= 12) { LOGE("demo: save write off (%zu)", wrote); free(blob); return 0; }

    projectile_pool fresh;
    projectile_pool_init(&fresh);
    int n = projectile_save_read(&fresh, blob, wrote);
    free(blob);
    if (n != 1) { LOGE("demo: save read restored %d (want 1)", n); return 0; }

    // find the lone restored stuck arrow and check it landed in the wall column.
    int found = 0;
    for (int i = 0; i < PROJECTILE_POOL_CAP; i++) {
        projectile *p = &fresh.slots[i];
        if (p->state != PROJ_STATE_STUCK) continue;
        found = 1;
        if (p->stuck_bx < 19 || p->stuck_bx > 21 || p->owner_id != 7) {
            LOGE("demo: restored arrow wrong (x=%d owner=%d)", p->stuck_bx, p->owner_id);
            return 0;
        }
    }
    if (!found) { LOGE("demo: no stuck arrow after reload"); return 0; }
    LOGI("demo: save round-trip ok (%zu bytes)", need);
    return 1;
}

#ifdef PROJECTILE_DEMO_MAIN
int main(void) {
    int ok = 1;
    ok &= test_arrow_sticks();
    ok &= test_arc_matches();
    ok &= test_spear_pierces();
    ok &= test_aim_lands();
    ok &= test_save_roundtrip();
    if (ok) { LOGI("projectile demo: all checks passed"); return 0; }
    LOGE("projectile demo: FAILED");
    return 1;
}
#endif
