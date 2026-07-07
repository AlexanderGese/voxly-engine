#include "gbuffer.h"
#include "../../util/log.h"

#include <string.h>

// full 4x4 inverse via cofactors. we only ever feed it the projection
// matrix which is well-conditioned, but keep it general so set_camera can
// stay dumb. if the matrix is singular we fall back to identity and the
// reconstruction shader produces garbage, which is loud enough to notice.
static mat4 mat4_inverse_local(mat4 src) {
    const float *m = &src.m[0][0];
    float inv[16], det;

    inv[0]  =  m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15]
             + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
    inv[4]  = -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15]
             - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
    inv[8]  =  m[4]*m[9]*m[15] - m[4]*m[11]*m[13] - m[8]*m[5]*m[15]
             + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
    inv[12] = -m[4]*m[9]*m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14]
             - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
    inv[1]  = -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15]
             - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
    inv[5]  =  m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15]
             + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
    inv[9]  = -m[0]*m[9]*m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15]
             - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
    inv[13] =  m[0]*m[9]*m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14]
             + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
    inv[2]  =  m[1]*m[6]*m[15] - m[1]*m[7]*m[14] - m[5]*m[2]*m[15]
             + m[5]*m[3]*m[14] + m[13]*m[2]*m[7] - m[13]*m[3]*m[6];
    inv[6]  = -m[0]*m[6]*m[15] + m[0]*m[7]*m[14] + m[4]*m[2]*m[15]
             - m[4]*m[3]*m[14] - m[12]*m[2]*m[7] + m[12]*m[3]*m[6];
    inv[10] =  m[0]*m[5]*m[15] - m[0]*m[7]*m[13] - m[4]*m[1]*m[15]
             + m[4]*m[3]*m[13] + m[12]*m[1]*m[7] - m[12]*m[3]*m[5];
    inv[14] = -m[0]*m[5]*m[14] + m[0]*m[6]*m[13] + m[4]*m[1]*m[14]
             - m[4]*m[2]*m[13] - m[12]*m[1]*m[6] + m[12]*m[2]*m[5];
    inv[3]  = -m[1]*m[6]*m[11] + m[1]*m[7]*m[10] + m[5]*m[2]*m[11]
             - m[5]*m[3]*m[10] - m[9]*m[2]*m[7] + m[9]*m[3]*m[6];
    inv[7]  =  m[0]*m[6]*m[11] - m[0]*m[7]*m[10] - m[4]*m[2]*m[11]
             + m[4]*m[3]*m[10] + m[8]*m[2]*m[7] - m[8]*m[3]*m[6];
    inv[11] = -m[0]*m[5]*m[11] + m[0]*m[7]*m[9] + m[4]*m[1]*m[11]
             - m[4]*m[3]*m[9] - m[8]*m[1]*m[7] + m[8]*m[3]*m[5];
    inv[15] =  m[0]*m[5]*m[10] - m[0]*m[6]*m[9] - m[4]*m[1]*m[10]
             + m[4]*m[2]*m[9] + m[8]*m[1]*m[6] - m[8]*m[2]*m[5];

    det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
    if (det == 0.0f) {
        LOGE("gbuffer: singular proj matrix, position reconstruction broken");
        return mat4_identity();
    }
    det = 1.0f / det;

    mat4 out;
    for (int i = 0; i < 16; i++) (&out.m[0][0])[i] = inv[i] * det;
    return out;
}

static int make_accum_buffer(gbuffer *g, int w, int h) {
    glGenFramebuffers(1, &g->accum_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, g->accum_fbo);

    glGenTextures(1, &g->accum_tex);
    glBindTexture(GL_TEXTURE_2D, g->accum_tex);
    // 16f so additive blending of many lights doesn't band/clip
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, g->accum_tex, 0);

    int ok = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (!ok) LOGE("gbuffer: accum framebuffer incomplete");
    return ok;
}

int gbuffer_init(gbuffer *g, int w, int h) {
    memset(g, 0, sizeof *g);
    g->w = w;
    g->h = h;
    g->view_mode = GBUFFER_VIEW_FINAL;
    g->ambient[0] = g->ambient[1] = g->ambient[2] = 0.08f;

    if (!gbuffer_target_create(&g->target, w, h)) return 0;
    if (!make_accum_buffer(g, w, h)) return 0;

    g->prog_geometry = gl_load_shader("shaders/gbuffer_geom.vert",
                                      "shaders/gbuffer_geom.frag");
    g->prog_accum    = gl_load_shader("shaders/gbuffer_fs.vert",
                                      "shaders/gbuffer_accum.frag");
    g->prog_shade    = gl_load_shader("shaders/gbuffer_fs.vert",
                                      "shaders/gbuffer_shade.frag");
    g->prog_debug    = gl_load_shader("shaders/gbuffer_fs.vert",
                                      "shaders/gbuffer_debug.frag");

    if (!g->prog_geometry || !g->prog_accum || !g->prog_shade || !g->prog_debug) {
        LOGE("gbuffer_init: shader load failed");
        gbuffer_shutdown(g);
        return 0;
    }

    // fullscreen triangle gets generated in the vertex shader from gl_VertexID
    // so the vao is empty but still needs to exist to draw.
    glGenVertexArrays(1, &g->fs_vao);

    g->view = mat4_identity();
    g->proj = mat4_identity();
    g->inv_proj = mat4_identity();
    g->culled.count = 0;
    return 1;
}

void gbuffer_shutdown(gbuffer *g) {
    gbuffer_target_destroy(&g->target);
    if (g->accum_fbo) glDeleteFramebuffers(1, &g->accum_fbo);
    if (g->accum_tex) glDeleteTextures(1, &g->accum_tex);
    if (g->fs_vao)    glDeleteVertexArrays(1, &g->fs_vao);
    gl_delete_shader(g->prog_geometry);
    gl_delete_shader(g->prog_accum);
    gl_delete_shader(g->prog_shade);
    gl_delete_shader(g->prog_debug);
    g->accum_fbo = g->accum_tex = g->fs_vao = 0;
}

void gbuffer_resize(gbuffer *g, int w, int h) {
    if (g->w == w && g->h == h) return;
    g->w = w;
    g->h = h;
    gbuffer_target_resize(&g->target, w, h);
    if (g->accum_fbo) glDeleteFramebuffers(1, &g->accum_fbo);
    if (g->accum_tex) glDeleteTextures(1, &g->accum_tex);
    make_accum_buffer(g, w, h);
}

void gbuffer_set_camera(gbuffer *g, mat4 view, mat4 proj) {
    g->view = view;
    g->proj = proj;
    g->inv_proj = mat4_inverse_local(proj);
}

void gbuffer_set_ambient(gbuffer *g, vec3 ambient) {
    g->ambient[0] = ambient.x;
    g->ambient[1] = ambient.y;
    g->ambient[2] = ambient.z;
}

void gbuffer_set_view_mode(gbuffer *g, gbuffer_view_mode m) {
    if (m < 0 || m >= GBUFFER_VIEW_COUNT) m = GBUFFER_VIEW_FINAL;
    g->view_mode = m;
}
