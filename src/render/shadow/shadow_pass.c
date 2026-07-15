#include "shadow_pass.h"
#include "../../util/log.h"

int shadow_pass_init(shadow_pass *sp) {
    sp->prog_depth = gl_load_shader("shaders/shadow_depth.vert",
                                    "shaders/shadow_depth.frag");
    if (!sp->prog_depth) {
        LOGE("shadow_pass: depth shader failed to load");
        return 0;
    }
    // we look the uniform up by name on each draw via gl_set_uniform_mat4, so
    // we dont actually need a cached location, but keep the field for the day
    // i decide the string hashing is too slow (it isnt).
    sp->u_view_proj = 0;
    sp->cull_front  = 1;
    return 1;
}

void shadow_pass_shutdown(shadow_pass *sp) {
    gl_delete_shader(sp->prog_depth);
    sp->prog_depth = 0;
}

void shadow_pass_render(shadow_pass *sp, shadow_map *sm,
                        const shadow_csm *csm,
                        shadow_caster_fn draw, void *ctx) {
    if (!csm->enabled || !draw) return;

    glUseProgram(sp->prog_depth);

    // slope-scaled polygon offset pushes the depth away from the light to
    // kill self-shadowing acne. magic numbers tuned by squinting.
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(SHADOW_DEPTH_BIAS_SLOPE, SHADOW_DEPTH_BIAS_CONST);

    // rendering back faces into the depth map trades acne for a little peter
    // panning, which the normal-offset bias in the main pass then hides.
    GLint prev_cull;
    glGetIntegerv(GL_CULL_FACE_MODE, &prev_cull);
    if (sp->cull_front) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
    }

    for (int i = 0; i < csm->count; i++) {
        shadow_map_begin_layer(sm, i);

        mat4 vp = csm->cascade[i].view_proj;
        gl_set_uniform_mat4(sp->prog_depth, "u_light_vp", mat4_data(&vp));

        // hand control to the caller to actually emit geometry for cascade i.
        draw(i, vp, ctx);
    }

    shadow_map_end(sm);

    // restore the bits we touched
    glCullFace(prev_cull);
    glDisable(GL_POLYGON_OFFSET_FILL);
}
