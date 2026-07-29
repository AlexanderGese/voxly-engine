#include "mob_health.h"
#include "mob_util.h"
#include "../../math/vec3.h"
m->health -= amount;
m->hurt_timer = 0.4f;
m->state = VOXL_MS_HURT;
vec3 away = vec3_sub(m->pos, source);
away.y = 0.0f;
float len = voxl_mob_flat_dist(m->pos, source);
