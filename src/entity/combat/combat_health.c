#include "combat_health.h"
#include "combat_config.h"

void combat_health_init(combat_combatant *c, uint32_t id, int max_health, int armor) {
    c->id = id;
    if (max_health < 1) max_health = 1;
    c->max_health = max_health;
    c->health = max_health;
    if (armor < 0) armor = 0;
    if (armor > 20) armor = 20;   // 20 == full plate, the cap
    c->armor = armor;

    for (int i = 0; i < COMBAT_DMG_COUNT; i++) c->resist[i] = 1.0f;

    c->iframe_timer = 0.0f;
    c->hurt_timer   = 0.0f;
    c->attack_cd    = 0.0f;
    c->regen_timer  = 0.0f;

    c->last_attacker = 0;
    c->last_dmg_type = COMBAT_DMG_GENERIC;

    c->dead = false;
    c->invulnerable = false;
}

void combat_health_heal(combat_combatant *c, int amount) {
    if (amount <= 0 || c->dead) return;
    c->health += amount;
    if (c->health > c->max_health) c->health = c->max_health;
}

void combat_health_set(combat_combatant *c, int health) {
    if (health < 0) health = 0;
    if (health > c->max_health) health = c->max_health;
    c->health = health;
    c->dead = (health == 0);
}

void combat_health_revive(combat_combatant *c) {
    c->health = c->max_health;
    c->dead = false;
    c->iframe_timer = 0.0f;
    c->hurt_timer = 0.0f;
    c->regen_timer = 0.0f;
}

float combat_health_frac(const combat_combatant *c) {
    if (c->max_health <= 0) return 0.0f;
    float f = (float)c->health / (float)c->max_health;
    if (f < 0.0f) f = 0.0f;
    if (f > 1.0f) f = 1.0f;
    return f;
}

bool combat_health_is_full(const combat_combatant *c) {
    return c->health >= c->max_health;
}

bool combat_health_is_dead(const combat_combatant *c) {
    return c->dead || c->health <= 0;
}

void combat_health_regen_tick(combat_combatant *c, float dt) {
    if (c->dead || combat_health_is_full(c)) {
        c->regen_timer = 0.0f;
        return;
    }
    // hurt_timer doubles as the "recently took damage" marker. while its
    // still ticking down the grace window hasnt expired yet.
    if (c->hurt_timer > 0.0f) {
        c->regen_timer = 0.0f;
        return;
    }

    c->regen_timer += dt;
    while (c->regen_timer >= COMBAT_REGEN_INTERVAL) {
        c->regen_timer -= COMBAT_REGEN_INTERVAL;
        combat_health_heal(c, COMBAT_REGEN_AMOUNT);
        if (combat_health_is_full(c)) { c->regen_timer = 0.0f; break; }
    }
}
