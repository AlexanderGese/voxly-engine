#include "projectile_hit.h"

#include <math.h>

// has this entity id already been damaged by this projectile? we only track the
// low 32 ids in a bitmask, which is a cheap dedupe for piercing spears. ids
// above 31 just never dedupe — acceptable, a spear hitting two high-id mobs in
// the same line is rare and the worst case is it double-hits. dont over-engineer.
static int already_hit(uint32_t mask, int id) {
    if (id < 0 || id >= 32) return 0;
    return (mask >> id) & 1u;
}

int projectile_hit_targets(projectile_kind kind, int owner_id,
                           uint32_t hit_mask_lo,
                           vec3 from, vec3 to,
                           const projectile_target *targets, int count,
                           projectile_entity_hit *out) {
    vec3 dir = vec3_sub(to, from);
    float seg = vec3_length(dir);
    if (seg < 1e-6f || count <= 0) return 0;

    vec3 rd = vec3_scale(dir, 1.0f / seg);
    float r = projectile_kind_def(kind)->radius;

    int    best_i = -1;
    float  best_t = seg + 1.0f;
    vec3   best_pt = VEC3_ZERO;

    for (int i = 0; i < count; i++) {
        const projectile_target *tg = &targets[i];
        if (tg->id == owner_id) continue;               // no self-damage
        if (already_hit(hit_mask_lo, tg->id)) continue; // pierce dedupe

        // inflate the target box by the projectile radius and ray-test it. this
        // is the standard minkowski trick: fat ray vs box == thin ray vs box
        // grown by the radius. good enough; we dont need a true capsule sweep.
        aabb fat = aabb_expand(tg->box, (vec3){ r, r, r });

        float tmin;
        if (!aabb_ray(fat, from, rd, &tmin)) continue;
        if (tmin < 0.0f || tmin > seg) continue;        // hit is off-segment
        if (tmin >= best_t) continue;

        best_t = tmin;
        best_i = i;
        best_pt = vec3_add(from, vec3_scale(rd, tmin));
    }

    if (best_i < 0) return 0;
    out->id    = targets[best_i].id;
    out->t     = best_t / seg;
    out->point = best_pt;
    return 1;
}

int projectile_hit_damage(projectile_kind kind, float speed, float speed0) {
    const projectile_def *d = projectile_kind_def(kind);
    if (d->base_damage <= 0) return 0;

    // damage scales with the fraction of muzzle speed retained. a near-spent
    // arrow that limps into a mob barely tickles. floor the ratio so a freak
    // tiny speed0 doesnt blow up the division.
    float ratio = (speed0 > 1e-3f) ? (speed / speed0) : 0.0f;
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;

    float dmg = (float)d->base_damage * ratio;

    // crit: above the threshold speed, a flat 50% bonus. arrows-at-full-draw
    // feeling chunky is half the fun.
    if (speed >= d->crit_speed) dmg *= 1.5f;

    int out = (int)(dmg + 0.5f);
    // a hit that landed at all should do at least 1 if the kind deals damage.
    if (out < 1 && ratio > 0.05f) out = 1;
    return out;
}
