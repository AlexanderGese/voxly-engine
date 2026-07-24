#include "entity_render.h"
#include "../math/mat4.h"
#include "../util/log.h"

// cube verts (positions only, -0.5..0.5 box at origin)
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
};

int entity_renderer_init(entity_renderer *er) {
    er->prog = gl_load_shader("shaders/entity.vert", "shaders/entity.frag");
    if (!er->prog) {
        LOGE("entity shader load failed");
        return 0;
    }
    glGenVertexArrays(1, &er->vao);
    glGenBuffers(1, &er->vbo);
    glBindVertexArray(er->vao);
    glBindBuffer(GL_ARRAY_BUFFER, er->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof cube, cube, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glBindVertexArray(0);
    return 1;
}

void entity_renderer_destroy(entity_renderer *er) {
    if (er->vao) glDeleteVertexArrays(1, &er->vao);
    if (er->vbo) glDeleteBuffers(1, &er->vbo);
    gl_delete_shader(er->prog);
}

static void color_for(entity_type t, float *r, float *g, float *b) {
    switch (t) {
    case ET_ZOMBIE:   *r = 0.2f; *g = 0.6f; *b = 0.2f; break;
    case ET_COW:      *r = 0.5f; *g = 0.3f; *b = 0.1f; break;
    case ET_PIG:      *r = 1.0f; *g = 0.6f; *b = 0.6f; break;
    case ET_SKELETON: *r = 0.9f; *g = 0.9f; *b = 0.9f; break;
    case ET_SPIDER:   *r = 0.1f; *g = 0.1f; *b = 0.1f; break;
    default:          *r = 1.0f; *g = 0.0f; *b = 1.0f; break;
    }
}

void entity_renderer_draw(entity_renderer *er, mob_registry *mr, const camera *cam) {
    mat4 view = camera_view(cam);
    mat4 proj = camera_proj(cam);

    glUseProgram(er->prog);
    gl_set_uniform_mat4(er->prog, "u_view", mat4_data(&view));
    gl_set_uniform_mat4(er->prog, "u_proj", mat4_data(&proj));
    glBindVertexArray(er->vao);

    for (int i = 0; i < MAX_MOBS; i++) {
        entity *e = &mr->list[i];
        if (!e->alive) continue;
        float w = entity_width(e->type);
        float h = entity_height(e->type);
        vec3 center = (vec3){ e->pos.x, e->pos.y + h * 0.5f, e->pos.z };
        mat4 t = mat4_translate(center);
        mat4 s = mat4_scale((vec3){w, h, w});
        mat4 model = mat4_mul(t, s);
        gl_set_uniform_mat4(er->prog, "u_model", mat4_data(&model));
        float r, g, b;
        color_for(e->type, &r, &g, &b);
        if (e->hurt_timer > 0) { r = 1; g = 0.2f; b = 0.2f; }
        gl_set_uniform_vec3(er->prog, "u_color", r, g, b);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}
