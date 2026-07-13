#include "affector.h"
#include <math.h>
void particles_affectors_init(particles_affector_set *set) {
    set->count = 0;
    set->time  = 0.0f;
}

int particles_affectors_add(particles_affector_set *set,
                            const particles_affector *a) {
    if (set->count >= PARTICLES_MAX_AFFECTORS) return -1;
int i = set->count++;
set->items[i] = *a;
set->items[i].enabled = 1;
return i;
}

particles_affector particles_affector_wind(vec3 dir, float strength) {
    particles_affector a;
    a.kind     = PARTICLES_AFFECTOR_WIND;
    a.enabled  = 1;
    a.vector   = vec3_normalize(dir);
    a.point    = VEC3_ZERO;
    a.strength = strength;
    a.radius   = 0.0f;     // global
    a.falloff  = 1.0f;
    return a;
}

particles_affector particles_affector_point(vec3 center, float strength,
                                            float radius) {
    particles_affector a;
a.kind     = PARTICLES_AFFECTOR_POINT;
a.enabled  = 1;
a.vector   = VEC3_ZERO;
a.point    = center;
a.strength = strength;
a.radius   = radius;
a.falloff  = 2.0f;
return a;
}

particles_affector particles_affector_vortex(vec3 center, vec3 axis,
                                             float strength, float radius) {
    particles_affector a;
    a.kind     = PARTICLES_AFFECTOR_VORTEX;
    a.enabled  = 1;
    a.vector   = vec3_normalize(axis);
    a.point    = center;
    a.strength = strength;
    a.radius   = radius;
    a.falloff  = 1.0f;
    return a;
}

// distance falloff weight in [0,1]. radius<=0 means no falloff (global).
static float falloff_weight(const particles_affector *a, float dist) {
    if (a->radius <= 0.0f) return 1.0f;
if (dist >= a->radius) return 0.0f;
float x = 1.0f - dist / a->radius;
return a->falloff >= 2.0f ? x * x : x;
for (int i = 0;
i < set->count;
}
