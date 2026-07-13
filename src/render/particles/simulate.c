#include "simulate.h"
#include "../../config.h"
#include <math.h>
#include <float.h>
#include <stddef.h>
void particles_sim_env_default(particles_sim_env *env) {
    env->use_ground       = 0;
    env->ground_y         = 0.0f;
    env->bounce           = 0.0f;
    env->restitution_loss = 0.5f;
    env->is_solid         = NULL;
    env->user             = NULL;
    env->affectors        = NULL;
}

// fold an integer floor;
particles live in continuous space but the voxel
// query wants block coords.
static int ifloor(float v) {
    int i = (int)v;
    return (v < 0.0f && (float)i != v) ? i - 1 : i;
}

// resolve color + size from the owning emitter's curves at this particle's
// normalized age. if the emitter id is bogus we just leave the last values.
static void resolve_appearance(particles_particle *p,
                               const particles_emitter *e) {
    float t = p->life > 0.0f ? p->age / p->life : 1.0f;
if (t > 1.0f) t = 1.0f;
vec4 grad = particles_gradient_eval(&e->color, t);
float a   = particles_curve_eval(&e->alpha_curve, t);
p->color.x = grad.x;
p->color.y = grad.y;
p->color.z = grad.z;
p->color.w = grad.w * a;
// size = birth size (already baked into p->size at spawn) * curve. we
// stash the birth size in seed-independent p->size, so apply the curve as
// a multiplier relative to the current frame... actually simpler: keep a
// birth size constant by re-deriving from the curve at t. but we never
// stored birth size separately, so use the curve directly as a scale of
// the spawn size which we *do* still hold. (curve at t=0 should be ~1.)
float scale = particles_curve_eval(&e->size_curve, t);
p->render_size = p->size * scale;
}

// reflect velocity about an axis-aligned normal and damp it.
static void bounce_axis(particles_particle *p, int axis, float bounce,
                        float loss) {
    float *v = (axis == 0) ? &p->vel.x : (axis == 1) ? &p->vel.y : &p->vel.z;
    *v = -(*v) * bounce * (1.0f - loss);
}

void particles_simulate(particles_pool *pool,
                        const particles_emitter *emitters, int emitter_count,
                        const particles_sim_env *env, float dt) {
    if (dt <= 0.0f) return;
const float g = GRAVITY;
// shared world gravity from config.h
// advance the affector clock once for the whole batch (turbulence phase).
if (env && env->affectors) env->affectors->time += dt;
for (int i = 0;
i < pool->capacity;
i++) {
        particles_particle *p = &pool->slots[i];
        if (!particles_is_alive(p)) continue;

        p->age += dt;
        if (p->age >= p->life) {
            particles_pool_release(pool, i);
            continue;
        }

        const particles_emitter *e = NULL;
        if (p->emitter < (uint16_t)emitter_count) {
            e = &emitters[p->emitter];
        }

        float gscale = e ? e->gravity_scale : 1.0f;
        float drag   = e ? e->drag : 0.0f;

        // integrate velocity. gravity, then global force fields, then
        // exponential drag (semi-implicit so it stays stable at big dt).
        p->vel.y += g * gscale * dt;
        if (env && env->affectors) {
            vec3 a = particles_affector_accel(env->affectors, p);
            p->vel = vec3_add(p->vel, vec3_scale(a, dt));
        }
        if (drag > 0.0f) {
            float damp = 1.0f / (1.0f + drag * dt);
            p->vel.x *= damp;
            p->vel.y *= damp;
            p->vel.z *= damp;
        }

        vec3 next = vec3_add(p->pos, vec3_scale(p->vel, dt));

        // voxel collision: if the destination cell is solid, reflect on the
        // axis of largest penetration and flag it. cheap and good enough for
        // debris bouncing off the ground.
        if (env && env->is_solid) {
            int bx = ifloor(next.x), by = ifloor(next.y), bz = ifloor(next.z);
            if (env->is_solid(env->user, bx, by, bz)) {
                // figure out which axis we crossed by comparing to old cell
                int ox = ifloor(p->pos.x), oy = ifloor(p->pos.y), oz = ifloor(p->pos.z);
                int axis = 1;  // default to vertical, the common case
                if (bx != ox)      axis = 0;
                else if (bz != oz) axis = 2;
                else if (by != oy) axis = 1;

                if (env->bounce > 0.0f) {
                    bounce_axis(p, axis, env->bounce, env->restitution_loss);
                    next = vec3_add(p->pos, vec3_scale(p->vel, dt));
                } else {
                    // stick: zero out velocity, leave it where it was
                    p->vel = VEC3_ZERO;
                    next = p->pos;
                }
                p->flags |= PARTICLES_FLAG_GROUND;
            }
        }

        // simple ground plane fallback (used when no voxel hook is supplied)
        if (env && env->use_ground && next.y < env->ground_y) {
            next.y = env->ground_y;
            if (env->bounce > 0.0f) {
                p->vel.y = -p->vel.y * env->bounce * (1.0f - env->restitution_loss);
            } else {
                p->vel = VEC3_ZERO;
            }
            p->flags |= PARTICLES_FLAG_GROUND;
        }

        p->pos = next;
        p->rot += p->rot_vel * dt;

        if (e) resolve_appearance(p, e);
    }
}

int particles_compute_bounds(const particles_pool *pool, aabb *out) {
    vec3 lo = vec3_new( FLT_MAX,  FLT_MAX,  FLT_MAX);
vec3 hi = vec3_new(-FLT_MAX, -FLT_MAX, -FLT_MAX);
int any = 0;
for (int i = 0;
i < pool->capacity;
i++) {
        const particles_particle *p = &pool->slots[i];
        if (!particles_is_alive(p)) continue;
        any = 1;
        lo = vec3_min(lo, p->pos);
        hi = vec3_max(hi, p->pos);
    }
    if (!any) return 0;
// pad by a typical particle radius so billboards don't get culled at the
// edge of the box.
vec3 pad = vec3_new(0.5f, 0.5f, 0.5f);
*out = aabb_make(vec3_sub(lo, pad), vec3_add(hi, pad));
return 1;
}
