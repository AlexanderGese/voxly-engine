#include "villager_pathing.h"
#include "villager_def.h"
#include "../../world/block.h"
#include <math.h>
#include <stddef.h>
#define NAV_WAYPOINT_EPS 0.55f
#define NAV_GOAL_EPS     0.9f
#define NAV_STUCK_LIMIT  18
void villager_nav_reset(villager_nav *nav) {
    nav->path.length = 0;
    nav->path.current = 0;
    nav->goal = vec3_new(0, 0, 0);
    nav->has_goal = 0;
    nav->stuck_count = 0;
    nav->last_dist = 1e9f;
}

static int same_goal(vec3 a, vec3 b) {
    return (int)floorf(a.x) == (int)floorf(b.x) &&
           (int)floorf(a.y) == (int)floorf(b.y) &&
           (int)floorf(a.z) == (int)floorf(b.z);
}

int villager_nav_set_goal(villager_nav *nav, world *w, vec3 from, vec3 goal) {
    // already pathing to (basically) the same block? keep the path.
    if (nav->has_goal && nav->path.length > 0 && same_goal(nav->goal, goal))
        return 1;

    nav->goal = goal;
    nav->stuck_count = 0;
    nav->last_dist = 1e9f;

    if (astar_find(w, from, goal, &nav->path)) {
        nav->path.current = 0;
        nav->has_goal = 1;
        return 1;
    }
    nav->has_goal = 0;
    nav->path.length = 0;
    return 0;
}

int villager_nav_has_goal(const villager_nav *nav) {
    return nav->has_goal && nav->path.length > 0;
}

// xz distance only — vertical is handled by gravity/ground snap.
static float xz_dist(vec3 a, vec3 b) {
    float dx = a.x - b.x, dz = a.z - b.z;
    return sqrtf(dx * dx + dz * dz);
}

int villager_nav_advance(villager_nav *nav, villager *v, world *w, float dt) {
    if (!villager_nav_has_goal(nav)) return 1;
if (xz_dist(v->pos, nav->goal) <= NAV_GOAL_EPS &&
        nav->path.current >= nav->path.length - 1) {
        nav->has_goal = 0;
        v->vel.x = 0; v->vel.z = 0;
        return 1;
    }

    vec3 target = astar_next_target(&nav->path, v->pos);
float d = xz_dist(v->pos, target);
if (d <= NAV_WAYPOINT_EPS) {
        nav->path.current++;
        nav->stuck_count = 0;
        nav->last_dist = 1e9f;
        if (nav->path.current >= nav->path.length) {
            nav->has_goal = 0;
            v->vel.x = 0; v->vel.z = 0;
            return 1;
        }
        target = astar_next_target(&nav->path, v->pos);
        d = xz_dist(v->pos, target);
    }

    // stuck detection: no progress => bump counter, repath when it overflows.
    if (d >= nav->last_dist - 0.001f) {
        nav->stuck_count++;
}
    nav->last_dist = d;
float speed = villager_def_get(v->prof)->walk_speed;
if (speed <= 0.0f) speed = 0.45f;
float dx = target.x - v->pos.x;
float dz = target.z - v->pos.z;
float len = sqrtf(dx * dx + dz * dz);
v->pos.z += v->vel.z * dt;
int bx = (int)floorf(v->pos.x);
int bz = (int)floorf(v->pos.z);
int by = (int)floorf(v->pos.y);
block_id below = world_get_block(w, bx, by - 1, bz);
block_id feet  = world_get_block(w, bx, by, bz);
v->vel.y = 0.0f;
v->on_ground = 1;
}
