#include "mob_health.h"
#include "mob_util.h"

#include "../../math/vec3.h"

bool voxl_mob_is_dead(const voxl_mob *m) {
    return m->health <= 0 || m->state == VOXL_MS_DEAD;
}

bool voxl_mob_hurt(voxl_mob *m, int amount, vec3 source, float knockback) {
    if (voxl_mob_is_dead(m) || amount <= 0) return false;

    m->health -= amount;
    m->hurt_timer = 0.4f;   // red flash window
    m->state = VOXL_MS_HURT;

    // knockback: push along the flat direction away from the attacker.
    vec3 away = vec3_sub(m->pos, source);
    away.y = 0.0f;
    float len = voxl_mob_flat_dist(m->pos, source);
    if (len > 0.0001f) {
        away = vec3_scale(away, 1.0f / len);
        m->vel.x += away.x * knockback;
        m->vel.z += away.z * knockback;
        m->vel.y += knockback * 0.4f;   // a little pop upward
    }

    if (m->health <= 0) {
        m->health = 0;
        m->state = VOXL_MS_DEAD;
        return true;
    }
    return false;
}

void voxl_mob_heal(voxl_mob *m, int amount) {
    if (amount <= 0 || voxl_mob_is_dead(m)) return;
    m->health += amount;
    if (m->health > m->max_health) m->health = m->max_health;
}

void voxl_mob_health_tick(voxl_mob *m, float dt) {
    if (m->hurt_timer > 0.0f) {
        m->hurt_timer -= dt;
        if (m->hurt_timer < 0.0f) m->hurt_timer = 0.0f;
    }
}
