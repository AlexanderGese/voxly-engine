#include "projectile_render.h"
#include "projectile_stick.h"
#include "../../math/mat4.h"
#include "../../util/log.h"
#include <math.h>
static const float cube[] = {
    -.5f,-.5f,-.5f,  .5f,-.5f,-.5f,  .5f, .5f,-.5f,
    -.5f,-.5f,-.5f,  .5f, .5f,-.5f, -.5f, .5f,-.5f,
    -.5f,-.5f, .5f,  .5f,-.5f, .5f,  .5f, .5f, .5f,
    -.5f,-.5f, .5f,  .5f, .5f, .5f, -.5f, .5f, .5f,
    -.5f, .5f,-.5f,  .5f, .5f,-.5f,  .5f, .5f, .5f,
    -.5f, .5f,-.5f,  .5f, .5f, .5f, -.5f, .5f, .5f,
    -.5f,-.5f,-.5f,  .5f,-.5f,-.5f,  .5f,-.5f, .5f,
    -.5f,-.5f,-.5f,  .5f,-.5f, .5f, -.5f,-.5f, .5f,
    -.5f,-.5f,-.5f, -.5f, .5f,-.5f, -.5f, .5f, .5f,
    -.5f,-.5f,-.5f, -.5f, .5f, .5f, -.5f,-.5f, .5f,
     .5f,-.5f,-.5f,  .5f, .5f,-.5f,  .5f, .5f, .5f,
     .5f,-.5f,-.5f,  .5f, .5f, .5f,  .5f,-.5f, .5f,
}
;
int projectile_renderer_init(projectile_renderer *pr) {
    pr->ready = 0;
    // reuse the entity shader: positions in, flat color out. no point authoring
    // a second trivial pipeline for sticks.
    pr->prog = gl_load_shader("shaders/entity.vert", "shaders/entity.frag");
    if (!pr->prog) {
        LOGE("projectile shader load failed");
        return 0;
    }
    glGenVertexArrays(1, &pr->vao);
    glGenBuffers(1, &pr->vbo);
    glBindVertexArray(pr->vao);
    glBindBuffer(GL_ARRAY_BUFFER, pr->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof cube, cube, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glBindVertexArray(0);
    pr->ready = 1;
    return 1;
}

void projectile_renderer_destroy(projectile_renderer *pr) {
    if (pr->vao) glDeleteVertexArrays(1, &pr->vao);
if (pr->vbo) glDeleteBuffers(1, &pr->vbo);
gl_delete_shader(pr->prog);
pr->ready = 0;
}

// orient the box so its +z axis points along forward. we build the basis by hand
// (look_at gives a *view* matrix, the inverse of what we want for a model) so we
// just drop the basis vectors into the columns. roll about the heading by spin.
mat4 projectile_render_orient(vec3 forward, vec3 pos, float spin, vec3 scale) {
    vec3 f = vec3_normalize(forward);
    vec3 ref = (fabsf(f.y) < 0.99f) ? VEC3_UP : VEC3_RIGHT;
    vec3 right = vec3_normalize(vec3_cross(ref, f));
    vec3 up    = vec3_cross(f, right);

    // roll the right/up pair around f for the fletching spin.
    float c = cosf(spin), s = sinf(spin);
    vec3 r2 = vec3_add(vec3_scale(right, c), vec3_scale(up, s));
    vec3 u2 = vec3_add(vec3_scale(up, c), vec3_scale(right, -s));

    mat4 m = mat4_identity();
    // columns: x=right, y=up, z=forward, translation in the 4th.
    m.m[0][0] = r2.x * scale.x; m.m[0][1] = r2.y * scale.x; m.m[0][2] = r2.z * scale.x;
    m.m[1][0] = u2.x * scale.y; m.m[1][1] = u2.y * scale.y; m.m[1][2] = u2.z * scale.y;
    m.m[2][0] = f.x  * scale.z; m.m[2][1] = f.y  * scale.z; m.m[2][2] = f.z  * scale.z;
    m.m[3][0] = pos.x; m.m[3][1] = pos.y; m.m[3][2] = pos.z;
    return m;
}

// per-kind stick color so you can tell a spear from an arrow at a glance.
static void color_for(projectile_kind k, float *r, float *g, float *b) {
    switch (k) {
    case PROJECTILE_ARROW:    *r = 0.85f;
*g = 0.80f;
*b = 0.65f;
break;
case PROJECTILE_BOLT:     *r = 0.55f;
*g = 0.55f;
*b = 0.60f;
break;
case PROJECTILE_ROCK:     *r = 0.45f;
*g = 0.45f;
*b = 0.45f;
break;
case PROJECTILE_SNOWBALL: *r = 0.95f;
*g = 0.97f;
*b = 1.00f;
break;
case PROJECTILE_SPEAR:    *r = 0.60f;
*g = 0.40f;
*b = 0.20f;
break;
default:                  *r = 1.0f;
*g = 0.0f;
*b = 1.0f;
break;
