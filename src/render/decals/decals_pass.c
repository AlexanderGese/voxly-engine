#include "decals_pass.h"
#include "decals_blend.h"
#include "decals_config.h"
#include "../../util/log.h"

#include <string.h>

// build the cube->world matrix straight from the projector basis, no inverse
// needed: columns are the scaled basis vectors, translation is the center. the
// cube is [-0.5,0.5] so the scale is 2*half along each axis.
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
// shader can test; we only encode the bits the fs actually consults.
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
    if (!decals_program_build(&pass->prog)) {
        decals_mesh_destroy(&pass->mesh);
        return 0;
    }
    LOGI("decals: pass initialised");
    return 1;
}

void decals_pass_shutdown(decals_pass *pass) {
    decals_program_destroy(&pass->prog);
    decals_mesh_destroy(&pass->mesh);
}

void decals_pass_set_camera(decals_pass *pass, mat4 view_proj, mat4 inv_view_proj) {
    pass->view_proj     = view_proj;
    pass->inv_view_proj = inv_view_proj;
}

void decals_pass_set_screen(decals_pass *pass, int w, int h) {
    pass->screen_w = w;
    pass->screen_h = h;
}

// set the blend/depth state the stamp needs. we read the g-buffer depth as a
// texture (not as the bound depth attachment) so depth testing is off; the
// fragment discards handle the volume clipping instead.
static void begin_state(void) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    // dont cull: the camera can be inside a projector box, in which case the
    // front faces are behind us and only the back faces cover the screen.
    glDisable(GL_CULL_FACE);
}

static void end_state(void) {
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}

int decals_pass_draw(decals_pass *pass, const decals_pool *pool,
                     const decals_cull_result *vis, const decals_atlas *atlas,
                     glid depth_tex, glid gnormal_tex) {
    pass->drawn_last = 0;
    if (vis->count <= 0) return 0;
    if (!pass->prog.prog) return 0;   // program failed to build, no-op

    decals_program_use(&pass->prog);
    glUniformMatrix4fv(pass->prog.u_view_proj, 1, GL_FALSE,
                       mat4_data(&pass->view_proj));
    glUniformMatrix4fv(pass->prog.u_inv_view_proj, 1, GL_FALSE,
                       mat4_data(&pass->inv_view_proj));
    if (pass->prog.u_screen >= 0)
        glUniform2f(pass->prog.u_screen, (float)pass->screen_w, (float)pass->screen_h);
    if (pass->prog.u_normal_strength >= 0)
        glUniform1f(pass->prog.u_normal_strength, DECALS_NORMAL_BLEND_MAX);

    // bind the g-buffer reads + atlas to the units the program expects.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depth_tex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gnormal_tex);
    decals_atlas_bind(atlas, 2);

    begin_state();

    // walk the visible list in BATCH_MAX chunks. per-decal tint differs, so we
    // push the tint uniform per chunk using the first decal's tint — close
    // enough; tint-heavy decals are rare and the chunks stay coherent because
    // the cull sort groups by priority. (a cleaner pass would atlas the tint.)
    static decals_mesh_inst staging[DECALS_BATCH_MAX];
    int total = 0;

    for (int base = 0; base < vis->count; base += DECALS_BATCH_MAX) {
        int n = vis->count - base;
        if (n > DECALS_BATCH_MAX) n = DECALS_BATCH_MAX;

        const decals_decal *first = &pool->slots[vis->items[base].slot];
        if (pass->prog.u_tint >= 0)
            glUniform3f(pass->prog.u_tint,
                        first->tint[0], first->tint[1], first->tint[2]);

        // pick the batch's blend mode (alpha vs additive) from its decals'
        // flags. glow batches add into the g-buffer instead of compositing.
        decals_blend_apply(decals_blend_for_batch(pool, vis->items, base, n));

        for (int i = 0; i < n; i++) {
            const decals_decal *d = &pool->slots[vis->items[base + i].slot];
            fill_instance(&staging[i], d);
        }

        decals_mesh_upload(&pass->mesh, staging, n);
        decals_mesh_draw(&pass->mesh, n);
        total += n;
    }

    end_state();
    pass->drawn_last = total;
    return total;
}
