#include "combat_invuln.h"
#include "combat_config.h"
#include "combat_damagetype.h"

bool combat_invuln_active(const combat_combatant *c) {
    return c->iframe_timer > 0.0f;
}

void combat_invuln_trigger(combat_combatant *c) {
    // refresh both windows. dont let a stale longer window get shortened.
    if (c->iframe_timer < COMBAT_IFRAME_TIME) c->iframe_timer = COMBAT_IFRAME_TIME;
    if (c->hurt_timer   < COMBAT_HURT_TIME)   c->hurt_timer   = COMBAT_HURT_TIME;
}

bool combat_invuln_can_take_hit(const combat_combatant *c, combat_damage_type t) {
    if (c->dead) return false;
    if (c->invulnerable) return false;          // god mode eats everything

    // some sources (fire, void, drown) tick straight through iframes.
    if (combat_dmg_ignores_iframes(t)) return true;

    return !combat_invuln_active(c);
}

bool combat_invuln_is_hurt(const combat_combatant *c) {
    return c->hurt_timer > 0.0f;
}

bool combat_cooldown_ready(const combat_combatant *c) {
    return c->attack_cd <= 0.0f;
}

void combat_cooldown_start(combat_combatant *c, float seconds) {
    if (seconds < 0.0f) seconds = 0.0f;
    // dont stack cooldowns, just set the longest pending one.
    if (seconds > c->attack_cd) c->attack_cd = seconds;
}

void combat_invuln_tick(combat_combatant *c, float dt) {
    if (dt < 0.0f) dt = 0.0f;

    if (c->iframe_timer > 0.0f) {
        c->iframe_timer -= dt;
        if (c->iframe_timer < 0.0f) c->iframe_timer = 0.0f;
    }
    if (c->hurt_timer > 0.0f) {
        c->hurt_timer -= dt;
        if (c->hurt_timer < 0.0f) c->hurt_timer = 0.0f;
    }
    if (c->attack_cd > 0.0f) {
        c->attack_cd -= dt;
        if (c->attack_cd < 0.0f) c->attack_cd = 0.0f;
    }
}
