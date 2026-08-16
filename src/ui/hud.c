#include "hud.h"
#include "../config.h"
#include "../world/block.h"

static glid vao, vbo;
static int  ready = 0;

static void upload_verts(const float *verts, int count) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, count * 2 * sizeof(float), verts, GL_DYNAMIC_DRAW);
}

// rough color for each block type so the hotbar shows something
static void block_color(block_id id, float *r, float *g, float *b) {
    switch (id) {
    case BLOCK_STONE:    *r=0.50f; *g=0.50f; *b=0.50f; break;
    case BLOCK_DIRT:     *r=0.45f; *g=0.30f; *b=0.15f; break;
    case BLOCK_GRASS:    *r=0.30f; *g=0.65f; *b=0.20f; break;
    case BLOCK_SAND:     *r=0.85f; *g=0.80f; *b=0.55f; break;
    case BLOCK_WOOD:     *r=0.50f; *g=0.35f; *b=0.15f; break;
    case BLOCK_LEAVES:   *r=0.15f; *g=0.50f; *b=0.10f; break;
    case BLOCK_PLANKS:   *r=0.70f; *g=0.55f; *b=0.30f; break;
    case BLOCK_COBBLE:   *r=0.40f; *g=0.40f; *b=0.40f; break;
    case BLOCK_BEDROCK:  *r=0.15f; *g=0.15f; *b=0.15f; break;
    case BLOCK_GLASS:    *r=0.75f; *g=0.85f; *b=0.95f; break;
    case BLOCK_WATER:    *r=0.20f; *g=0.40f; *b=0.80f; break;
    case BLOCK_TORCH:    *r=1.00f; *g=0.85f; *b=0.20f; break;
    case BLOCK_BRICK:    *r=0.65f; *g=0.30f; *b=0.25f; break;
    case BLOCK_SNOW:     *r=0.95f; *g=0.95f; *b=0.95f; break;
    case BLOCK_ICE:      *r=0.60f; *g=0.80f; *b=0.95f; break;
    default:             *r=0.80f; *g=0.00f; *b=0.80f; break; // magenta = unknown
    }
}

void hud_init(void) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glBindVertexArray(0);
    ready = 1;
}

void hud_draw(glid prog, const inventory *inv, int sw, int sh) {
    if (!ready) hud_init();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(prog);
    gl_set_uniform_float(prog, "u_sw", (float)sw);
    gl_set_uniform_float(prog, "u_sh", (float)sh);

    glBindVertexArray(vao);

    // crosshair
    float cx = sw * 0.5f;
    float cy = sh * 0.5f;
    float sz = 10.0f;
    float cross[] = {
        cx - sz, cy,     cx + sz, cy,
        cx,      cy - sz, cx,     cy + sz,
    };
    upload_verts(cross, 4);
    gl_set_uniform_vec3(prog, "u_color", 1, 1, 1);
    glDrawArrays(GL_LINES, 0, 4);

    // hotbar
    int slots = HOTBAR_SLOTS;
    float slot_w = 44.0f;
    float gap    = 4.0f;
    float pad    = 4.0f;
    float total  = slots * slot_w + (slots - 1) * gap;
    float sx     = cx - total * 0.5f;
    float sy     = sh - 60.0f;

    for (int i = 0; i < slots; i++) {
        float x0 = sx + i * (slot_w + gap);
        float y0 = sy;
        float x1 = x0 + slot_w;
        float y1 = y0 + slot_w;

        // filled background (dark translucent)
        float bg[] = {
            x0, y0, x1, y0, x1, y1,
            x0, y0, x1, y1, x0, y1,
        };
        upload_verts(bg, 6);
        gl_set_uniform_vec3(prog, "u_color", 0.1f, 0.1f, 0.1f);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // block color fill inside
        block_id id = inv->slot[i];
        if (id != BLOCK_AIR) {
            float r, g, b;
            block_color(id, &r, &g, &b);
            float inner[] = {
                x0+pad, y0+pad, x1-pad, y0+pad, x1-pad, y1-pad,
                x0+pad, y0+pad, x1-pad, y1-pad, x0+pad, y1-pad,
            };
            upload_verts(inner, 6);
            gl_set_uniform_vec3(prog, "u_color", r, g, b);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // border
        float border[] = {
            x0, y0, x1, y0,
            x1, y0, x1, y1,
            x1, y1, x0, y1,
            x0, y1, x0, y0,
        };
        upload_verts(border, 8);
        if (i == inv->selected) gl_set_uniform_vec3(prog, "u_color", 1, 1, 0);
        else                    gl_set_uniform_vec3(prog, "u_color", 0.7f, 0.7f, 0.7f);
        glLineWidth(i == inv->selected ? 3.0f : 1.0f);
        glDrawArrays(GL_LINES, 0, 8);
    }

    glLineWidth(1.0f);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}
