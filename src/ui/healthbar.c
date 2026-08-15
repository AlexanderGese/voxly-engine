#include "healthbar.h"
#include "../config.h"

static glid vao, vbo;
static int  ready = 0;

static void ensure_init(void) {
    if (ready) return;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glBindVertexArray(0);
    ready = 1;
}

static void upload(const float *v, int n) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, n * 2 * sizeof(float), v, GL_DYNAMIC_DRAW);
}

static void draw_bar(glid prog, float x, float y, float w, float h,
                     float fill, float r, float g, float b, int sw, int sh) {
    // background
    float bg[] = {
        x, y, x+w, y, x+w, y+h,
        x, y, x+w, y+h, x, y+h,
    };
    upload(bg, 6);
    gl_set_uniform_vec3(prog, "u_color", 0.15f, 0.15f, 0.15f);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // fill
    float fw = w * fill;
    if (fw > 0.5f) {
        float fg[] = {
            x, y, x+fw, y, x+fw, y+h,
            x, y, x+fw, y+h, x, y+h,
        };
        upload(fg, 6);
        gl_set_uniform_vec3(prog, "u_color", r, g, b);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    // border
    float bd[] = {
        x, y, x+w, y,
        x+w, y, x+w, y+h,
        x+w, y+h, x, y+h,
        x, y+h, x, y,
    };
    upload(bd, 8);
    gl_set_uniform_vec3(prog, "u_color", 0.4f, 0.4f, 0.4f);
    glDrawArrays(GL_LINES, 0, 8);
}

void healthbar_draw(glid prog, const survival *s, int sw, int sh) {
    ensure_init();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(prog);
    gl_set_uniform_float(prog, "u_sw", (float)sw);
    gl_set_uniform_float(prog, "u_sh", (float)sh);
    glBindVertexArray(vao);

    float cx = sw * 0.5f;
    float bar_w = 180.0f;
    float bar_h = 10.0f;
    float bar_gap = 4.0f;
    float base_y = sh - 110.0f;

    // health bar (red)
    float hp_fill = (float)s->health / (float)MAX_HEALTH;
    draw_bar(prog, cx - bar_w - 10, base_y, bar_w, bar_h,
             hp_fill, 0.85f, 0.15f, 0.15f, sw, sh);

    // hunger bar (brown/orange)
    float hunger_fill = (float)s->hunger / (float)MAX_HUNGER;
    draw_bar(prog, cx + 10, base_y, bar_w, bar_h,
             hunger_fill, 0.75f, 0.55f, 0.15f, sw, sh);

    // stamina bar (green, below health)
    float stam_fill = s->stamina / (float)MAX_STAMINA;
    draw_bar(prog, cx - bar_w - 10, base_y + bar_h + bar_gap, bar_w * 0.6f, bar_h * 0.6f,
             stam_fill, 0.2f, 0.75f, 0.3f, sw, sh);

    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}
