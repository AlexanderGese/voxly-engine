#include "projectile_types.h"

#include <stddef.h>

// the kind table. one row per projectile_kind, indexed by the enum. add a kind,
// add a row, nothing else cares. numbers are eyeballed and then tweaked until
// arrows felt right against a wall at 20 blocks. dont read too much into them.

static const projectile_def k_defs[PROJECTILE_KIND_COUNT] = {
    [PROJECTILE_NONE] = {
        .kind = PROJECTILE_NONE, .name = "none",
        .mass = 1.0f, .radius = 0.1f, .drag = 0.0f, .gravity_scale = 1.0f,
        .restitution = 0.0f, .speed = 0.0f,
        .sticks = 0, .stick_depth = 0.0f, .stick_decay = 0.0f,
        .base_damage = 0, .crit_speed = 9999.0f, .max_lifetime = 0.0f,
        .gravity_immune = 0,
    },
    [PROJECTILE_ARROW] = {
        .kind = PROJECTILE_ARROW, .name = "arrow",
        .mass = 0.10f, .radius = 0.10f, .drag = 0.01f, .gravity_scale = 1.0f,
        .restitution = 0.0f, .speed = 38.0f,
        .sticks = 1, .stick_depth = 0.30f, .stick_decay = 60.0f,
        .base_damage = 6, .crit_speed = 34.0f, .max_lifetime = 30.0f,
        .gravity_immune = 0,
    },
    [PROJECTILE_BOLT] = {
        .kind = PROJECTILE_BOLT, .name = "bolt",
        .mass = 0.14f, .radius = 0.09f, .drag = 0.006f, .gravity_scale = 0.85f,
        .restitution = 0.0f, .speed = 46.0f,
        .sticks = 1, .stick_depth = 0.28f, .stick_decay = 45.0f,
        .base_damage = 8, .crit_speed = 40.0f, .max_lifetime = 25.0f,
        .gravity_immune = 0,
    },
    [PROJECTILE_ROCK] = {
        .kind = PROJECTILE_ROCK, .name = "rock",
        .mass = 0.5f, .radius = 0.18f, .drag = 0.02f, .gravity_scale = 1.0f,
        .restitution = 0.35f, .speed = 22.0f,
        .sticks = 0, .stick_depth = 0.0f, .stick_decay = 0.0f,
        .base_damage = 3, .crit_speed = 24.0f, .max_lifetime = 12.0f,
        .gravity_immune = 0,
    },
    [PROJECTILE_SNOWBALL] = {
        .kind = PROJECTILE_SNOWBALL, .name = "snowball",
        .mass = 0.2f, .radius = 0.15f, .drag = 0.04f, .gravity_scale = 0.9f,
        .restitution = 0.0f, .speed = 20.0f,
        .sticks = 0, .stick_depth = 0.0f, .stick_decay = 0.0f,
        .base_damage = 0, .crit_speed = 9999.0f, .max_lifetime = 8.0f,
        .gravity_immune = 0,
    },
    [PROJECTILE_SPEAR] = {
        .kind = PROJECTILE_SPEAR, .name = "spear",
        .mass = 1.6f, .radius = 0.12f, .drag = 0.008f, .gravity_scale = 1.1f,
        .restitution = 0.0f, .speed = 30.0f,
        .sticks = 1, .stick_depth = 0.55f, .stick_decay = 120.0f,
        .base_damage = 11, .crit_speed = 28.0f, .max_lifetime = 40.0f,
        .gravity_immune = 0,
    },
};

const projectile_def *projectile_kind_def(projectile_kind k) {
    if (k < 0 || k >= PROJECTILE_KIND_COUNT) return &k_defs[PROJECTILE_NONE];
    return &k_defs[k];
}

aabb projectile_kind_bounds(projectile_kind k) {
    float r = projectile_kind_def(k)->radius;
    vec3 h = (vec3){ r, r, r };
    return aabb_from_center(VEC3_ZERO, h);
}
