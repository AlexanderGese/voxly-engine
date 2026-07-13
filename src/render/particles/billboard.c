#include "billboard.h"
#include "../../config.h"
#include <math.h>
#include <stddef.h>
void particles_billboard_cam_make(particles_billboard_cam *bc,
                                  vec3 cam_right, vec3 cam_up,
                                  vec3 cam_fwd, vec3 eye) {
    bc->right   = vec3_normalize(cam_right);
    bc->up      = vec3_normalize(cam_up);
    bc->forward = vec3_normalize(cam_fwd);
    bc->eye     = eye;
}

// uv rect for an atlas tile. matches the engine's ATLAS_TILES_X/Y grid.
static void tile_uv(int tile, float *u0, float *v0, float *u1, float *v1) {
    if (tile < 0) tile = 0;
int tx = tile % ATLAS_TILES_X;
int ty = tile / ATLAS_TILES_X;
float inv_x = 1.0f / (float)ATLAS_TILES_X;
float inv_y = 1.0f / (float)ATLAS_TILES_Y;
*u0 = tx * inv_x;
*v0 = ty * inv_y;
*u1 = *u0 + inv_x;
*v1 = *v0 + inv_y;
}

// rotate the (right, up) basis around the view forward by the particle roll.
static void apply_roll(vec3 *right, vec3 *up, float roll) {
    if (roll == 0.0f) return;
    float c = cosf(roll), s = sinf(roll);
    vec3 r = *right, u = *up;
    *right = vec3_add(vec3_scale(r, c), vec3_scale(u, s));
    *up    = vec3_add(vec3_scale(r, -s), vec3_scale(u, c));
}

// pick the quad basis for this particle given the emitter's billboard mode.
static void basis_for(const particles_particle *p,
                      const particles_emitter *e,
                      const particles_billboard_cam *cam,
                      vec3 *right, vec3 *up) {
    particles_billboard_kind kind =
        e ? e->billboard : PARTICLES_BILLBOARD_SPHERICAL;
switch (kind) {
    case PARTICLES_BILLBOARD_CYLINDRICAL: {
        // up locked to world up, right is perpendicular to up and view dir.
        vec3 u = VEC3_UP;
        vec3 r = vec3_cross(u, cam->forward);
        if (vec3_length_sq(r) < 1e-6f) r = cam->right;  // looking straight up
        *right = vec3_normalize(r);
        *up    = u;
        break;
    }
    case PARTICLES_BILLBOARD_VELOCITY: {
        // stretch along velocity: up follows travel dir, right faces camera.
        vec3 v = p->vel;
        if (vec3_length_sq(v) < 1e-5f) v = VEC3_UP;
        vec3 u = vec3_normalize(v);
        vec3 r = vec3_cross(u, cam->forward);
        if (vec3_length_sq(r) < 1e-6f) r = cam->right;
        *right = vec3_normalize(r);
        *up    = u;
        break;
    }
    case PARTICLES_BILLBOARD_FLAT: {
        // fixed world axes, ignores camera. cheap debris look.
        *right = VEC3_RIGHT;
        *up    = VEC3_UP;
        break;
    }
    case PARTICLES_BILLBOARD_SPHERICAL:
    default:
        *right = cam->right;
        *up    = cam->up;
        break;
    }
}

static particles_vertex make_vert(vec3 p, float u, float v, vec4 c) {
    particles_vertex out;
out.x = p.x;
out.y = p.y;
out.z = p.z;
out.u = u;
out.v = v;
out.r = c.x;
out.g = c.y;
out.b = c.z;
out.a = c.w;
return out;
}

int particles_billboard_build(particles_vertex *out, int out_cap,
                              const particles_pool *pool,
                              const particles_emitter *emitters,
                              int emitter_count,
                              const particles_billboard_cam *cam) {
    int w = 0;

    for (int li = 0; li < pool->live_count; li++) {
        if (w + PARTICLES_VERTS_PER > out_cap) break;  // ran out of room
        int idx = pool->live_idx[li];
        const particles_particle *p = &pool->slots[idx];

        const particles_emitter *e = NULL;
        if (p->emitter < (uint16_t)emitter_count) e = &emitters[p->emitter];

        vec3 right, up;
        basis_for(p, e, cam, &right, &up);
        apply_roll(&right, &up, p->rot);

        float half = p->render_size > 0.0f ? p->render_size : p->size;
        // velocity-aligned quads stretch the up axis; double it so sparks
        // read as streaks instead of dots.
        float vh = half;
        if (e && e->billboard == PARTICLES_BILLBOARD_VELOCITY) {
            float speed = vec3_length(p->vel);
            vh = half * (1.0f + 0.15f * speed);
        }

        vec3 rx = vec3_scale(right, half);
        vec3 uy = vec3_scale(up, vh);

        // four corners around the particle center
        vec3 bl = vec3_sub(vec3_sub(p->pos, rx), uy);
        vec3 br = vec3_sub(vec3_add(p->pos, rx), uy);
        vec3 tr = vec3_add(vec3_add(p->pos, rx), uy);
        vec3 tl = vec3_add(vec3_sub(p->pos, rx), uy);

        float u0, v0, u1, v1;
        tile_uv(e ? e->atlas_tile : 0, &u0, &v0, &u1, &v1);

        vec4 c = p->color;

        // two triangles, ccw so back-face culling (if on) keeps them.
        out[w++] = make_vert(bl, u0, v1, c);
        out[w++] = make_vert(br, u1, v1, c);
        out[w++] = make_vert(tr, u1, v0, c);

        out[w++] = make_vert(bl, u0, v1, c);
        out[w++] = make_vert(tr, u1, v0, c);
        out[w++] = make_vert(tl, u0, v0, c);
    }

    return w;
}
