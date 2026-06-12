#ifndef MATH_FRUSTUM_H
#define MATH_FRUSTUM_H

#include "mat4.h"
#include "aabb.h"

// view frustum for chunk culling. extracted from view*proj.

typedef struct {
    // plane = (nx, ny, nz, d). point inside if n.p + d >= 0.
    float planes[6][4];
} frustum;

void frustum_from_matrix(frustum *f, mat4 vp);
int  frustum_contains_aabb(const frustum *f, aabb a);

#endif
