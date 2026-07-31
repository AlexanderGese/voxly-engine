#ifndef ENTITY_PROJECTILE_COLLIDE_H
#define ENTITY_PROJECTILE_COLLIDE_H

#include "projectile_types.h"
#include "projectile_sampler.h"

// block collision for a single flight segment. we sweep the tip from a->b with a
// voxel DDA (amanatides & woo) and report the first solid cell crossed plus the
// face we entered through. that face normal is what stick/bounce keys off of.

// which face of a voxel the ray entered. matches block.h face convention:
// 0=+x 1=-x 2=+y 3=-y 4=+z 5=-z. -1 means "started already inside a block".
typedef int projectile_face;

typedef struct {
    int hit;                // 1 if a solid block was crossed on this segment
    projectile_hitkind kind;// BLOCK or FLUID (fluid is a soft hit, no stop)
    int bx, by, bz;         // the block cell that was hit
    projectile_face face;   // face entered through, or -1
    vec3 point;             // world-space contact point on the segment
    vec3 normal;            // outward face normal at the contact
    float t;                // param along a->b in [0,1] where contact happened
} projectile_block_hit;

// march from `from` to `to`. on the first solid voxel, fills out and returns 1.
// fluid entry is reported too (kind == PROJ_HIT_FLUID) but march continues, so
// the *returned* hit is always the first hard stop unless the segment ends in
// fluid. returns 0 if the whole segment is clear air.
int projectile_collide_segment(const projectile_sampler *s,
                               vec3 from, vec3 to,
                               projectile_block_hit *out);

// outward normal for a face id. small lookup, exposed for the stick code.
vec3 projectile_face_normal(projectile_face f);

#endif
