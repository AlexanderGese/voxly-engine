#ifndef UTIL_BVH_RAYCAST_H
#define UTIL_BVH_RAYCAST_H
#include "bvh.h"
bvh_hit bvh_raycast(const bvh *b, vec3 origin, vec3 dir, float max_t);
#endif
