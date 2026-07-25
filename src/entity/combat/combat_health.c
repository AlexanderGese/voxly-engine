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
c->dead = false;
c->iframe_timer = 0.0f;
c->hurt_timer = 0.0f;
c->regen_timer = 0.0f;
return;
}
