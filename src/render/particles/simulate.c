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
float scale = particles_curve_eval(&e->size_curve, t);
p->render_size = p->size * scale;
const float g = GRAVITY;
if (env && env->affectors) env->affectors->time += dt;
for (int i = 0;
i < pool->capacity;
vec3 hi = vec3_new(-FLT_MAX, -FLT_MAX, -FLT_MAX);
int any = 0;
for (int i = 0;
i < pool->capacity;
vec3 pad = vec3_new(0.5f, 0.5f, 0.5f);
*out = aabb_make(vec3_sub(lo, pad), vec3_add(hi, pad));
return 1;
}
