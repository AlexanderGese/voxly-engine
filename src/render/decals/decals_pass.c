#include "decals_pass.h"
#include "decals_blend.h"
#include "decals_config.h"
#include "../../util/log.h"
#include <string.h>
static mat4 model_from_projector(const decals_projector *p) {
    mat4 m = mat4_identity();
    vec3 cx = vec3_scale(p->right,  2.0f * p->half.x);
    vec3 cy = vec3_scale(p->up,     2.0f * p->half.y);
    vec3 cz = vec3_scale(p->normal, 2.0f * p->half.z);
    // column-major: m[col][row].
    m.m[0][0] = cx.x; m.m[0][1] = cx.y; m.m[0][2] = cx.z;
    m.m[1][0] = cy.x; m.m[1][1] = cy.y; m.m[1][2] = cy.z;
    m.m[2][0] = cz.x; m.m[2][1] = cz.y; m.m[2][2] = cz.z;
    m.m[3][0] = p->center.x; m.m[3][1] = p->center.y; m.m[3][2] = p->center.z;
    return m;
}

// pack one decal into an instance payload. flags ride as a float bitfield the
// shader can test;
we only encode the bits the fs actually consults.
static void fill_instance(decals_mesh_inst *out, const decals_decal *d) {
    mat4 model = model_from_projector(&d->proj);
    memcpy(out->model,     mat4_data(&model),               16 * sizeof(float));
    memcpy(out->inv_model, mat4_data(&d->proj.world_to_local), 16 * sizeof(float));

    out->uv_rect[0] = d->region.uv0[0];
    out->uv_rect[1] = d->region.uv0[1];
    out->uv_rect[2] = d->region.uv1[0];
    out->uv_rect[3] = d->region.uv1[1];

    float flagbits = 0.0f;
    if (d->flags & DECALS_FLAG_NORMAL_MAP) flagbits += 1.0f;
    if (d->flags & DECALS_FLAG_ADDITIVE)   flagbits += 2.0f;
    if (d->flags & DECALS_FLAG_PROJECT_BACK) flagbits += 4.0f;

    out->params[0] = d->alpha;
    out->params[1] = d->angle_fade;
    out->params[2] = 0.5f;          // default roughness the decal writes
    out->params[3] = flagbits;
}

int decals_pass_init(decals_pass *pass) {
    memset(pass, 0, sizeof *pass);
pass->view_proj     = mat4_identity();
pass->inv_view_proj = mat4_identity();
if (!decals_mesh_create(&pass->mesh)) return 0;
return 1;
pass->inv_view_proj = inv_view_proj;
glDepthMask(GL_TRUE);
glEnable(GL_DEPTH_TEST);
