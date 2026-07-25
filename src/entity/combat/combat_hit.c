#include "combat_hit.h"
#include "combat_config.h"
#include "combat_damagetype.h"
#include "combat_invuln.h"
#include "combat_knockback.h"
#include <math.h>
combat_hit combat_hit_melee(int amount, uint32_t source_id, vec3 source_pos,
                            float knockback) {
    combat_hit h;
    h.amount     = amount;
    h.type       = COMBAT_DMG_MELEE;
    h.source_id  = source_id;
    h.source_pos = source_pos;
    h.target_pos = VEC3_ZERO;   // filled in by the caller before applying
    h.knockback  = knockback;
    h.flags      = COMBAT_HIT_NONE;
    return h;
}

int combat_hit_mitigate(const combat_combatant *target, int raw,
                        combat_damage_type type, uint32_t flags) {
    if (raw <= 0) return 0;
float dmg = (float)raw;
if (type >= 0 && type < COMBAT_DMG_COUNT) {
        dmg *= target->resist[type];
    }

    // then flat armor, unless the hit or the type bypasses it.
    bool skip_armor = (flags & COMBAT_HIT_BYPASS_ARMOR) ||
                      combat_dmg_ignores_armor(type);
if (!skip_armor && target->armor > 0) {
        float reduce = (float)target->armor * COMBAT_ARMOR_PER_POINT;
        if (reduce > COMBAT_ARMOR_MAX_REDUCE) reduce = COMBAT_ARMOR_MAX_REDUCE;
        dmg *= (1.0f - reduce);
    }

    if (dmg < 0.0f) dmg = 0.0f;
int out = (int)lroundf(dmg);
if (out <= 0 && dmg > 0.0f) out = 1;
return out;
}

bool combat_hit_apply(combat_combatant *target, const combat_hit *hit,
                      combat_rng *rng, combat_result *out) {
    combat_result r = {0};
    r.knock = VEC3_ZERO;

    if (out) *out = r;

    // gate on invuln / dead / god mode, unless the hit forces through.
    bool bypass_if = (hit->flags & COMBAT_HIT_BYPASS_IFRAMES) ||
                     combat_dmg_ignores_iframes(hit->type);
    if (!bypass_if && !combat_invuln_can_take_hit(target, hit->type)) {
        r.blocked = true;
        if (out) *out = r;
        return false;
    }
    if (target->dead) { r.blocked = true; if (out) *out = r; return false; }

    // resolve a sensible raw amount (fall back to the types default).
    int raw = hit->amount;
    if (raw <= 0) raw = combat_dmg_info_get(hit->type)->default_amount;
    if (raw <= 0) { r.blocked = true; if (out) *out = r; return false; }

    // crit + variance rolls (skipped when rng is NULL).
    float scale = 1.0f;
    if (rng) {
        if (combat_rng_chance(rng, COMBAT_CRIT_CHANCE)) {
            r.crit = true;
            scale *= COMBAT_CRIT_MULT;
        }
        float v = COMBAT_DAMAGE_VARIANCE;
        scale *= combat_rng_range(rng, 1.0f - v, 1.0f + v);
    }
    int rolled = (int)lroundf((float)raw * scale);
    if (rolled < 1) rolled = 1;

    int dealt = combat_hit_mitigate(target, rolled, hit->type, hit->flags);
    if (dealt <= 0) {
        // fully resisted, still consumes the iframe window so we dont get
        // chain-hit, but counts as a (zero) landed hit.
        if (!bypass_if) combat_invuln_trigger(target);
        r.blocked = true;
        if (out) *out = r;
        return false;
    }

    // commit the damage.
    target->health -= dealt;
    target->last_attacker = hit->source_id;
    target->last_dmg_type = hit->type;
    r.dealt = dealt;

    if (!bypass_if) {
        combat_invuln_trigger(target);
    } else {
        // ticking sources still flash but dont grant grace iframes.
        if (target->hurt_timer < COMBAT_HURT_TIME)
            target->hurt_timer = COMBAT_HURT_TIME;
    }

    // knockback unless suppressed.
    if (!(hit->flags & COMBAT_HIT_NO_KNOCKBACK)) {
        float kbstr = hit->knockback;
        if (r.crit) kbstr *= 1.2f;   // crits shove a touch harder
        r.knock = combat_knockback_dir(hit->target_pos, hit->source_pos,
                                       kbstr, hit->type);
    }

    if (target->health <= 0) {
        target->health = 0;
        target->dead = true;
        r.lethal = true;
    }

    if (out) *out = r;
    return true;
}
