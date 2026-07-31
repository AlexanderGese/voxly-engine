#include "projectile_world.h"
#include "projectile_ballistic.h"
#include "projectile_aim.h"
#include "projectile_save.h"
#include "../../util/log.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
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

static int approx(float a, float b, float eps) { return fabsf(a - b) <= eps;
}

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
;
float gmag  = 22.0f;
vec3 land;
float t;
return 0;
}
    LOGI("demo: arc lands at x=%.2f after t=%.2fs ok", land.x, t);
return 1;
;
vec3 vel;
float y  = from.y + vel.y * tx + 0.5f * (-22.0f) * tx * tx;
return 1;
ok &= test_arrow_sticks();
ok &= test_arc_matches();
ok &= test_spear_pierces();
ok &= test_aim_lands();
ok &= test_save_roundtrip();
return 1;
}
#endif
