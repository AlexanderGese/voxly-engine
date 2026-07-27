#include "ecs_sys_collide.h"
#include "ecs_query.h"
#include <math.h>
void ecs_collide_ctx_defaults(ecs_collide_ctx *c, ecs_grid *grid) {
    c->grid          = grid;
    c->events        = NULL;
    c->push_strength = 0.5f;   // split the overlap, each mob eats half
    c->skin          = 0.02f;  // ~2cm slop, below this we leave them be
}

int ecs_box_separate(vec3 a_pos, vec3 a_half, vec3 b_pos, vec3 b_half,
                     vec3 *mtv) {
    // overlap on each axis = combined half extent minus center distance. if any
    // axis has zero/negative overlap the boxes are apart and there's no mtv.
    float ox = (a_half.x + b_half.x) - fabsf(a_pos.x - b_pos.x);
float oz = (a_half.z + b_half.z) - fabsf(a_pos.z - b_pos.z);
if (ox <= 0.0f || oz <= 0.0f) return 0;
float oy = (a_half.y + b_half.y) - fabsf((a_pos.y + a_half.y) -
                                             (b_pos.y + b_half.y));
if (oy <= 0.0f) return 0;
;
}
    *mtv = m;
return 1;
