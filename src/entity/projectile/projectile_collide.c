#include "projectile_collide.h"

#include <math.h>

// face ids: 0=+x 1=-x 2=+y 3=-y 4=+z 5=-z. the normals point out of the block.
static const vec3 k_face_normal[6] = {
    {  1, 0, 0 }, { -1, 0, 0 },
    {  0, 1, 0 }, {  0,-1, 0 },
    {  0, 0, 1 }, {  0, 0,-1 },
};

vec3 projectile_face_normal(projectile_face f) {
    if (f < 0 || f > 5) return VEC3_ZERO;
    return k_face_normal[f];
}

// pick the entry face from the stepping axis and direction. we entered the new
// cell moving +x, so we came in through its -x face, etc.
static projectile_face entry_face(int axis, int step_dir) {
    switch (axis) {
        case 0: return step_dir > 0 ? 1 : 0;   // moved +x -> entered -x face
        case 1: return step_dir > 0 ? 3 : 2;   // moved +y -> entered -y face
        default: return step_dir > 0 ? 5 : 4;  // moved +z -> entered -z face
    }
}

int projectile_collide_segment(const projectile_sampler *s,
                               vec3 from, vec3 to,
                               projectile_block_hit *out) {
    out->hit = 0;
    out->kind = PROJ_HIT_NONE;
    out->face = -1;

    vec3 dir = vec3_sub(to, from);
    float seg_len = vec3_length(dir);
    if (seg_len < 1e-6f) return 0;          // didn't move, nothing to test

    // unit ray for the DDA; we'll convert the t-along-ray back to [0,1].
    vec3 rd = vec3_scale(dir, 1.0f / seg_len);

    int bx = (int)floorf(from.x);
    int by = (int)floorf(from.y);
    int bz = (int)floorf(from.z);

    // if we *start* embedded in a solid block, report it immediately with a
    // sentinel face. callers (e.g. a projectile spawned inside terrain) treat
    // this as an instant stick at the spawn cell.
    block_id here = projectile_sample(s, bx, by, bz);
    if (projectile_block_is_solid(here)) {
        out->hit = 1; out->kind = PROJ_HIT_BLOCK;
        out->bx = bx; out->by = by; out->bz = bz;
        out->face = -1; out->normal = VEC3_ZERO;
        out->point = from; out->t = 0.0f;
        return 1;
    }

    int step_x = rd.x > 0 ? 1 : (rd.x < 0 ? -1 : 0);
    int step_y = rd.y > 0 ? 1 : (rd.y < 0 ? -1 : 0);
    int step_z = rd.z > 0 ? 1 : (rd.z < 0 ? -1 : 0);

    // distance (in t-along-ray) to cross one cell on each axis.
    float inv_x = rd.x != 0 ? fabsf(1.0f / rd.x) : INFINITY;
    float inv_y = rd.y != 0 ? fabsf(1.0f / rd.y) : INFINITY;
    float inv_z = rd.z != 0 ? fabsf(1.0f / rd.z) : INFINITY;

    // t to reach the first cell boundary on each axis.
    float tx = inv_x;
    float ty = inv_y;
    float tz = inv_z;
    if (step_x > 0) tx = ((float)(bx + 1) - from.x) / rd.x;
    else if (step_x < 0) tx = (from.x - (float)bx) / -rd.x;
    if (step_y > 0) ty = ((float)(by + 1) - from.y) / rd.y;
    else if (step_y < 0) ty = (from.y - (float)by) / -rd.y;
    if (step_z > 0) tz = ((float)(bz + 1) - from.z) / rd.z;
    else if (step_z < 0) tz = (from.z - (float)bz) / -rd.z;

    int got_fluid = 0;
    float t = 0.0f;

    // walk cells until we pass the segment end.
    while (t <= seg_len) {
        int axis;
        float t_enter;
        if (tx <= ty && tx <= tz)      { axis = 0; t_enter = tx; bx += step_x; tx += inv_x; }
        else if (ty <= tz)             { axis = 1; t_enter = ty; by += step_y; ty += inv_y; }
        else                           { axis = 2; t_enter = tz; bz += step_z; tz += inv_z; }

        if (t_enter > seg_len) break;   // boundary is past b, segment is clear

        block_id id = projectile_sample(s, bx, by, bz);
        if (projectile_block_is_solid(id)) {
            int step_dir = (axis == 0) ? step_x : (axis == 1) ? step_y : step_z;
            projectile_face face = entry_face(axis, step_dir);
            out->hit = 1; out->kind = PROJ_HIT_BLOCK;
            out->bx = bx; out->by = by; out->bz = bz;
            out->face = face;
            out->normal = k_face_normal[face];
            out->point = vec3_add(from, vec3_scale(rd, t_enter));
            out->t = t_enter / seg_len;
            return 1;
        }
        if (!got_fluid && projectile_block_is_fluid(id)) {
            // remember the first fluid crossing; only surface it if no solid is
            // found later on the same segment.
            got_fluid = 1;
            out->kind = PROJ_HIT_FLUID;
            out->bx = bx; out->by = by; out->bz = bz;
            out->face = -1; out->normal = VEC3_ZERO;
            out->point = vec3_add(from, vec3_scale(rd, t_enter));
            out->t = t_enter / seg_len;
        }

        t = t_enter;
    }

    if (got_fluid) {
        out->hit = 1;       // soft hit; world loop checks kind to not stop
        return 1;
    }
    return 0;
}
