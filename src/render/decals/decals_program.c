#include "decals_program.h"
#include "decals_glsl.h"
#include "../../util/log.h"

#include <stddef.h>

// compile a single stage from source. logs the driver's gripe and returns 0 on
// failure. local copy rather than reaching into gl.c's static helper — keeps
// the subsystem self-contained, same as shaderman does.
static glid compile_stage(GLenum type, const char *src) {
    glid s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);

    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(s, sizeof log, NULL, log);
        LOGE("decals: %s shader compile failed: %s",
             type == GL_VERTEX_SHADER ? "vertex" : "fragment", log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

static int link_program(decals_program *p, glid vs, glid fs) {
    glid prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(prog, sizeof log, NULL, log);
        LOGE("decals: program link failed: %s", log);
        glDeleteProgram(prog);
        return 0;
    }
    p->prog = prog;
    return 1;
}

static void cache_uniforms(decals_program *p) {
    p->u_view_proj      = glGetUniformLocation(p->prog, "u_view_proj");
    p->u_inv_view_proj  = glGetUniformLocation(p->prog, "u_inv_view_proj");
    p->u_screen         = glGetUniformLocation(p->prog, "u_screen");
    p->u_depth          = glGetUniformLocation(p->prog, "u_depth");
    p->u_gnormal        = glGetUniformLocation(p->prog, "u_gnormal");
    p->u_atlas          = glGetUniformLocation(p->prog, "u_atlas");
    p->u_tint           = glGetUniformLocation(p->prog, "u_tint");
    p->u_normal_strength = glGetUniformLocation(p->prog, "u_normal_strength");
}

int decals_program_build(decals_program *p) {
    p->prog = 0;
    glid vs = compile_stage(GL_VERTEX_SHADER, DECALS_GLSL_VERT);
    if (!vs) return 0;
    glid fs = compile_stage(GL_FRAGMENT_SHADER, DECALS_GLSL_FRAG);
    if (!fs) { glDeleteShader(vs); return 0; }

    int linked = link_program(p, vs, fs);
    // stages are baked into the program after link; we can drop them either way.
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!linked) return 0;

    cache_uniforms(p);
    LOGI("decals: pass program built");
    return 1;
}

void decals_program_destroy(decals_program *p) {
    if (p->prog) glDeleteProgram(p->prog);
    p->prog = 0;
}

void decals_program_use(const decals_program *p) {
    glUseProgram(p->prog);
    // sampler bindings are static for the lifetime of the program; set them
    // here so the pass only deals with the per-frame state.
    if (p->u_depth   >= 0) glUniform1i(p->u_depth,   0);
    if (p->u_gnormal >= 0) glUniform1i(p->u_gnormal, 1);
    if (p->u_atlas   >= 0) glUniform1i(p->u_atlas,   2);
}
