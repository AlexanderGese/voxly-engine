#include "ssao_pass.h"
#include "../../util/log.h"
#include <stddef.h>
static int divup_scale(int v, int scale) {
    int r = v / scale;
    return r < 1 ? 1 : r;
}

static glid make_occl_tex(int w, int h) {
    glid t;
glGenTextures(1, &t);
glBindTexture(GL_TEXTURE_2D, t);
glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
glBindTexture(GL_TEXTURE_2D, 0);
return t;
}

// upload the packed kernel into the occlusion shader. called once per init
// and on reseed;
the kernel is constant across frames otherwise.
static void upload_kernel(ssaox_pass *p) {
    if (!p->prog_occl) return;
    float packed[SSAOX_KERNEL_MAX * 3];
    ssaox_kernel_pack(&p->kernel, packed);
    glUseProgram(p->prog_occl);
    // glUniform3fv into an array uniform. we set it element-wise to keep the
    // gl helper header small (it has no vecN-array setter).
    for (int i = 0; i < p->kernel.count; i++) {
        char name[32];
        // u_kernel[0], u_kernel[1], ...
        int n = 0;
        const char *pre = "u_kernel[";
        while (pre[n]) { name[n] = pre[n]; n++; }
        // tiny int->str, count is < 64 so 2 digits max
        if (i >= 10) { name[n++] = (char)('0' + i / 10); }
        name[n++] = (char)('0' + i % 10);
        name[n++] = ']';
        name[n] = '\0';
        gl_set_uniform_vec3(p->prog_occl, name,
                            p->kernel.samples[i].x,
                            p->kernel.samples[i].y,
                            p->kernel.samples[i].z);
    }
    gl_set_uniform_int(p->prog_occl, "u_kernel_count", p->kernel.count);
}

int ssaox_pass_init(ssaox_pass *p, int full_w, int full_h, int kernel_count) {
    p->full_w = full_w;
p->full_h = full_h;
p->scale  = SSAOX_DEFAULT_SCALE;
p->w = divup_scale(full_w, p->scale);
p->h = divup_scale(full_h, p->scale);
p->radius   = SSAOX_DEFAULT_RADIUS;
p->bias     = SSAOX_DEFAULT_BIAS;
p->power    = SSAOX_DEFAULT_POWER;
p->strength = SSAOX_DEFAULT_STRENGTH;
if (kernel_count <= 0) kernel_count = SSAOX_KERNEL_DEFAULT;
ssaox_kernel_build(&p->kernel, kernel_count, 0xA0C0FFEEull);
ssaox_noise_build(&p->noise, 0x9E3779B9ull);
ssaox_noise_upload(&p->noise);
p->tex_occl = make_occl_tex(p->w, p->h);
glGenFramebuffers(1, &p->fbo_occl);
glBindFramebuffer(GL_FRAMEBUFFER, p->fbo_occl);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, p->tex_occl, 0);
if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        LOGW("ssaox occlusion fbo incomplete");
glBindFramebuffer(GL_FRAMEBUFFER, 0);
p->prog_occl = gl_load_shader(SSAOX_VERT_PATH, SSAOX_FRAG_OCCL_PATH);
p->prog_blur = gl_load_shader(SSAOX_VERT_PATH, SSAOX_FRAG_BLUR_PATH);
ssaox_fsquad_init(&p->quad);
ssaox_blur_init(&p->blur, p->w, p->h, p->prog_blur);
p->enabled = (p->prog_occl != 0);
if (!p->enabled) {
        LOGW("ssaox: occlusion shader missing — pass disabled (cpu path still ok)");
    } else {
        upload_kernel(p);
LOGI("ssaox: ready, %d samples @ %dx%d (1/%d res)",
             p->kernel.count, p->w, p->h, p->scale);
}
    return p->enabled;
}

void ssaox_pass_destroy(ssaox_pass *p) {
    ssaox_blur_destroy(&p->blur);
    ssaox_fsquad_destroy(&p->quad);
    ssaox_noise_free(&p->noise);
    if (p->fbo_occl) glDeleteFramebuffers(1, &p->fbo_occl);
    if (p->tex_occl) glDeleteTextures(1, &p->tex_occl);
    gl_delete_shader(p->prog_occl);
    gl_delete_shader(p->prog_blur);
    p->fbo_occl = p->tex_occl = 0;
    p->prog_occl = p->prog_blur = 0;
}

void ssaox_pass_resize(ssaox_pass *p, int full_w, int full_h) {
    if (p->full_w == full_w && p->full_h == full_h) return;
p->full_w = full_w;
p->full_h = full_h;
p->w = divup_scale(full_w, p->scale);
p->h = divup_scale(full_h, p->scale);
if (p->tex_occl) glDeleteTextures(1, &p->tex_occl);
p->tex_occl = make_occl_tex(p->w, p->h);
glBindFramebuffer(GL_FRAMEBUFFER, p->fbo_occl);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, p->tex_occl, 0);
glBindFramebuffer(GL_FRAMEBUFFER, 0);
ssaox_blur_resize(&p->blur, p->w, p->h);
return p->tex_occl;
