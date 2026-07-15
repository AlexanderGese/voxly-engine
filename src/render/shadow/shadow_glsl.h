#ifndef RENDER_SHADOW_GLSL_H
#define RENDER_SHADOW_GLSL_H
extern const char *SHADOW_GLSL_DEPTH_VERT;
extern const char *SHADOW_GLSL_DEPTH_FRAG;
extern const char *SHADOW_GLSL_SAMPLE_LIB;
int shadow_glsl_write(const char *dir);
#endif
