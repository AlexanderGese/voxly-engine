#include "combat_explosion.h"
#include "combat_hit.h"
#include "combat_knockback.h"

#include <math.h>

combat_explosion combat_explosion_make(vec3 center, float power, float radius,
                                       uint32_t source_id) {
    combat_explosion b;
    b.center    = center;
    b.power     = power;
    b.radius    = radius;
    b.source_id = source_id;
    return b;
}

int combat_explosion_damage_at(const combat_explosion *b, vec3 pos, float exposure) {
    if (b->power <= 0.0f || b->radius <= 0.0f) return 0;
    if (exposure < 0.0f) exposure = 0.0f;
    if (exposure > 1.0f) exposure = 1.0f;

    vec3 d = vec3_sub(pos, b->center);
    float dist = sqrtf(d.x * d.x + d.y * d.y + d.z * d.z);
    if (dist >= b->radius) return 0;

    // squared falloff: t in [0,1], damage = power * (1-t)^2.
    float t = dist / b->radius;
    float falloff = (1.0f - t);
    falloff *= falloff;

    float dmg = b->power * falloff * exposure;
    int out = (int)lroundf(dmg);
    if (out <= 0 && dmg > 0.25f) out = 1;   // graze still counts
    return out;
}

bool combat_explosion_hit(const combat_explosion *b, combat_combatant *target,
                          vec3 target_pos, float exposure, combat_rng *rng,
                          combat_result *out) {
    int raw = combat_explosion_damage_at(b, target_pos, exposure);
    if (raw <= 0) {
        if (out) { combat_result z = {0}; z.knock = VEC3_ZERO; *out = z; }
        return false;
    }

    combat_hit h;
    h.amount     = raw;
    h.type       = COMBAT_DMG_EXPLOSION;
    h.source_id  = b->source_id;
    h.source_pos = b->center;
    h.target_pos = target_pos;
    h.knockback  = 0.0f;   // explosions use the radial model below, not dir
    h.flags      = COMBAT_HIT_NO_KNOCKBACK;  // we set knock ourselves

    combat_result r;
    bool dealt = combat_hit_apply(target, &h, rng, &r);

    // override with radial knockback scaled by the same exposure.
    if (dealt) {
        vec3 kb = combat_knockback_radial(target_pos, b->center,
                                          b->power, b->radius);
        r.knock = vec3_scale(kb, exposure);
    }

    if (out) *out = r;
    return dealt;
}
