#include "villager_pathing.h"
#include "villager_def.h"
#include "../../world/block.h"

#include <math.h>
#include <stddef.h>

// how close (xz) counts as "reached" a waypoint / the final goal.
#define NAV_WAYPOINT_EPS 0.55f
#define NAV_GOAL_EPS     0.9f

// if our distance to the current waypoint stops shrinking this many ticks in
// a row, assume we snagged on geometry and ask for a fresh path.
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

    // arrived at the final goal?
    if (xz_dist(v->pos, nav->goal) <= NAV_GOAL_EPS &&
        nav->path.current >= nav->path.length - 1) {
        nav->has_goal = 0;
        v->vel.x = 0; v->vel.z = 0;
        return 1;
    }

    vec3 target = astar_next_target(&nav->path, v->pos);
    float d = xz_dist(v->pos, target);

    // reached this waypoint, advance to the next.
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
        if (nav->stuck_count > NAV_STUCK_LIMIT) {
            // drop the path; brain will re-issue a goal next tick.
            villager_nav_reset(nav);
            return 0;
        }
    } else {
        nav->stuck_count = 0;
    }
    nav->last_dist = d;

    // seek the waypoint at the profession's walk speed.
    float speed = villager_def_get(v->prof)->walk_speed;
    if (speed <= 0.0f) speed = 0.45f;
    float dx = target.x - v->pos.x;
    float dz = target.z - v->pos.z;
    float len = sqrtf(dx * dx + dz * dz);
    if (len > 1e-4f) {
        v->vel.x = (dx / len) * speed;
        v->vel.z = (dz / len) * speed;
        v->yaw = atan2f(dx, dz);
    }

    // integrate xz.
    v->pos.x += v->vel.x * dt;
    v->pos.z += v->vel.z * dt;

    // cheap gravity + ground snap so they don't sink through the floor while
    // path-walking. the real physics module isn't wired to villagers yet.
    int bx = (int)floorf(v->pos.x);
    int bz = (int)floorf(v->pos.z);
    int by = (int)floorf(v->pos.y);
    block_id below = world_get_block(w, bx, by - 1, bz);
    block_id feet  = world_get_block(w, bx, by, bz);

    if (block_is_solid(feet)) {
        // walked into a step-up; nudge up one block (villagers auto-step).
        v->pos.y += 1.0f;
        v->vel.y = 0.0f;
        v->on_ground = 1;
    } else if (block_is_solid(below)) {
        v->pos.y = (float)by;
        v->vel.y = 0.0f;
        v->on_ground = 1;
    } else {
        // float down until we find ground.
        v->vel.y += -22.0f * dt;
        if (v->vel.y < -20.0f) v->vel.y = -20.0f;
        v->pos.y += v->vel.y * dt;
        v->on_ground = 0;
    }
    return 0;
}
