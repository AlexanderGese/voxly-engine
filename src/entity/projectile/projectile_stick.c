#include "projectile_stick.h"

#include <math.h>

// minimum speed into a face for a rock to bounce rather than just splat. below
// this it doesnt have the energy and we stop it, which also kills the case where
// a slow rock would jitter forever in a corner shedding a few percent each tap.
#define STICK_BOUNCE_MIN_SPEED  4.0f

static void do_stick(projectile *p, const projectile_block_hit *hit) {
    const projectile_def *d = projectile_kind_def(p->kind);

    p->state = PROJ_STATE_STUCK;
    p->vel   = VEC3_ZERO;
    p->stuck_bx = hit->bx;
    p->stuck_by = hit->by;
    p->stuck_bz = hit->bz;
    p->age = 0.0f;  // reuse age as the stuck-decay clock

    // bury the tip stick_depth past the face along the incoming heading, then
    // store the offset relative to the block origin so we can rebuild the tip
    // later without keeping a world pos that a chunk move would invalidate.
    vec3 into = p->forward;                 // heading at moment of impact
    vec3 tip  = vec3_add(hit->point, vec3_scale(into, d->stick_depth));

    p->stuck_off = (vec3){
        tip.x - (float)hit->bx,
        tip.y - (float)hit->by,
        tip.z - (float)hit->bz,
    };
    p->pos = tip;
}

projectile_resolve projectile_stick_resolve(projectile *p,
                                            const projectile_block_hit *hit) {
    const projectile_def *d = projectile_kind_def(p->kind);

    // started embedded (face -1): always just stick at the spawn cell if the
    // kind sticks, else stop. no bounce makes sense from inside a block.
    if (hit->face < 0) {
        if (d->sticks) { do_stick(p, hit); return PROJ_RESOLVE_STICK; }
        p->state = PROJ_STATE_SPENT; p->vel = VEC3_ZERO;
        return PROJ_RESOLVE_STOP;
    }

    if (d->sticks) {
        do_stick(p, hit);
        return PROJ_RESOLVE_STICK;
    }

    // bounce path (rocks). reflect velocity about the face normal and only keep
    // going if there's enough normal speed and the kind is bouncy.
    float vn = vec3_dot(p->vel, hit->normal);   // <0 means moving into the face
    if (d->restitution > 0.0f && vn < -STICK_BOUNCE_MIN_SPEED) {
        // v' = v - (1+e)(v·n)n
        vec3 reflect = vec3_sub(p->vel,
            vec3_scale(hit->normal, (1.0f + d->restitution) * vn));
        p->vel = reflect;
        // shove the tip just off the surface so next step doesnt re-detect this
        // same cell from inside its boundary.
        p->pos = vec3_add(hit->point, vec3_scale(hit->normal, 0.01f));
        return PROJ_RESOLVE_BOUNCE;
    }

    // not enough oomph (or not bouncy): dead stop at the contact point.
    p->pos = hit->point;
    p->vel = VEC3_ZERO;
    p->state = PROJ_STATE_SPENT;
    return PROJ_RESOLVE_STOP;
}

vec3 projectile_stick_tip(const projectile *p) {
    return (vec3){
        (float)p->stuck_bx + p->stuck_off.x,
        (float)p->stuck_by + p->stuck_off.y,
        (float)p->stuck_bz + p->stuck_off.z,
    };
}

int projectile_stick_decay(projectile *p, float dt) {
    const projectile_def *d = projectile_kind_def(p->kind);
    p->age += dt;
    return p->age >= d->stick_decay;
}
