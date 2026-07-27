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

    // y overlap matters too -- a mob standing on another's head isnt clipping.
    float oy = (a_half.y + b_half.y) - fabsf((a_pos.y + a_half.y) -
                                             (b_pos.y + b_half.y));
    if (oy <= 0.0f) return 0;

    // resolve along whichever horizontal axis needs the least travel. pushing
    // out the *minimum* axis keeps mobs from popping over walls of each other.
    vec3 m = {0, 0, 0};
    if (ox < oz) {
        m.x = (a_pos.x < b_pos.x) ? -ox : ox;
    } else {
        m.z = (a_pos.z < b_pos.z) ? -oz : oz;
    }
    *mtv = m;
    return 1;
}

void ecs_sys_collide(ecs_world *w, float dt, void *user) {
    (void)dt;
    ecs_collide_ctx *ctx = user;
    if (!ctx || !ctx->grid) return;

    ecs_query q;
    ecs_query_begin(&q, w,
                    ecs_with(ECS_CMP_TRANSFORM, ECS_CMP_COLLIDER, -1), 0);

    ecs_entity near[64];

    while (ecs_query_next(&q)) {
        ecs_transform *atf = ecs_query_get(&q, ECS_CMP_TRANSFORM);
        ecs_collider  *aco = ecs_query_get(&q, ECS_CMP_COLLIDER);

        // query a radius a touch bigger than this box so we catch a neighbour
        // whose center sits just outside but whose body still overlaps.
        float reach = (aco->half.x > aco->half.z ? aco->half.x : aco->half.z)
                      + (float)ECS_GRID_CELL * 0.5f;
        uint32_t n = ecs_grid_query_radius(ctx->grid, atf->pos, reach,
                                           q.e, near, 64);

        for (uint32_t i = 0; i < n; i++) {
            ecs_entity other = near[i];
            // only resolve each pair once: skip if the other index is lower, the
            // loop will (or already did) handle it from that entity's side.
            if (ecs_entity_index(other) < ecs_entity_index(q.e)) continue;

            ecs_transform *btf = ecs_get(w, other, ECS_CMP_TRANSFORM);
            ecs_collider  *bco = ecs_get(w, other, ECS_CMP_COLLIDER);
            if (!btf || !bco) continue;

            vec3 mtv;
            if (!ecs_box_separate(atf->pos, aco->half,
                                  btf->pos, bco->half, &mtv))
                continue;

            float mag = fabsf(mtv.x) + fabsf(mtv.z);
            if (mag < ctx->skin) continue;   // within slop, dont fight over it

            // split the push. scale by push_strength so it eases apart over a
            // couple frames instead of snapping (snapping looks like teleport).
            float s = ctx->push_strength;
            atf->pos.x -= mtv.x * s;
            atf->pos.z -= mtv.z * s;
            btf->pos.x += mtv.x * s;
            btf->pos.z += mtv.z * s;
        }
    }
}
