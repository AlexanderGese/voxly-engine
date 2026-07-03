#include "gl.h"
#include "../util/log.h"
#include "../util/file.h"

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int gl_init(void) {
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        LOGE("glad failed to load gl");
        return 0;
    }
    LOGI("gl version: %s", glGetString(GL_VERSION));
    LOGI("gl renderer: %s", glGetString(GL_RENDERER));
    return 1;
}

static glid compile_one(GLenum type, const char *src, const char *path) {
    glid s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);
    int ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(s, sizeof log, NULL, log);
        LOGE("shader compile %s: %s", path, log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

glid gl_load_shader(const char *vert_path, const char *frag_path) {
    size_t vs_len, fs_len;
    char *vs = file_read_all(vert_path, &vs_len);
    char *fs = file_read_all(frag_path, &fs_len);
    if (!vs || !fs) {
        LOGE("cant read shader files");
        free(vs); free(fs);
        return 0;
    }

    glid v = compile_one(GL_VERTEX_SHADER,   vs, vert_path);
    glid f = compile_one(GL_FRAGMENT_SHADER, fs, frag_path);
    free(vs); free(fs);
    if (!v || !f) {
        if (v) glDeleteShader(v);
        if (f) glDeleteShader(f);
        return 0;
    }

    glid prog = glCreateProgram();
    glAttachShader(prog, v);
    glAttachShader(prog, f);
    glLinkProgram(prog);
    int ok;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(prog, sizeof log, NULL, log);
        LOGE("shader link: %s", log);
        glDeleteProgram(prog);
        glDeleteShader(v);
        glDeleteShader(f);
        return 0;
    }
    glDeleteShader(v);
    glDeleteShader(f);
    return prog;
}

void gl_delete_shader(glid prog) {
    if (prog) glDeleteProgram(prog);
}

void gl_set_uniform_mat4(glid prog, const char *name, const float *m) {
    int loc = glGetUniformLocation(prog, name);
    glUniformMatrix4fv(loc, 1, GL_FALSE, m);
}

void gl_set_uniform_vec3(glid prog, const char *name, float x, float y, float z) {
    int loc = glGetUniformLocation(prog, name);
    glUniform3f(loc, x, y, z);
}

void gl_set_uniform_int(glid prog, const char *name, int v) {
    int loc = glGetUniformLocation(prog, name);
    glUniform1i(loc, v);
}

void gl_set_uniform_float(glid prog, const char *name, float v) {
    int loc = glGetUniformLocation(prog, name);
    glUniform1f(loc, v);
}

void gl_check_error_(const char *file, int line) {
    GLenum e = glGetError();
    if (e == GL_NO_ERROR) return;
    const char *name = "?";
    switch (e) {
    case GL_INVALID_ENUM:      name = "INVALID_ENUM"; break;
    case GL_INVALID_VALUE:     name = "INVALID_VALUE"; break;
    case GL_INVALID_OPERATION: name = "INVALID_OPERATION"; break;
    case GL_OUT_OF_MEMORY:     name = "OUT_OF_MEMORY"; break;
    }
    LOGW("gl error %s at %s:%d", name, file, line);
}
