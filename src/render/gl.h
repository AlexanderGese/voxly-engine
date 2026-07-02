#ifndef RENDER_GL_H
#define RENDER_GL_H

// tiny gl helper header. we use glad for loading but since this is
// a single-file project i put the gl header in third_party/ and the
// function loading is done via glfwGetProcAddress in render/gl.c

// NOTE: intentionally includes glcorearb so we get the core profile
// symbols without pulling in compatibility junk

#include <glad/glad.h>

typedef unsigned int glid;

int  gl_init(void);
glid gl_load_shader(const char *vert_path, const char *frag_path);
void gl_delete_shader(glid prog);

void gl_set_uniform_mat4(glid prog, const char *name, const float *m);
void gl_set_uniform_vec3(glid prog, const char *name, float x, float y, float z);
void gl_set_uniform_int (glid prog, const char *name, int v);
void gl_set_uniform_float(glid prog, const char *name, float v);

void gl_check_error_(const char *file, int line);
#define GL_CHECK() gl_check_error_(__FILE__, __LINE__)

#endif
