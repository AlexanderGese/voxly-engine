#ifndef RENDER_SHADOW_GLSL_H
#define RENDER_SHADOW_GLSL_H
// the shadow shaders, as source strings. the rest of the engine loads shaders
// from disk via gl_load_shader, but the depth pass is tiny and i got tired of
// the shaders/ dir drifting out of sync with the uniform names here, so the
// canonical copy lives next to the code that feeds it. shadow_glsl_write()
// stamps these to disk so gl_load_shader still finds them.
// trivial depth-only vertex shader: clip = light_vp * model_pos. no normals,
// no uvs, the fragment shader is empty (depth is written automatically).
extern const char *SHADOW_GLSL_DEPTH_VERT;
extern const char *SHADOW_GLSL_DEPTH_FRAG;
extern const char *SHADOW_GLSL_SAMPLE_LIB;
int shadow_glsl_write(const char *dir);
#endif
