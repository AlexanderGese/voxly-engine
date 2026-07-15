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

// the sampling routine the main lighting shader #includes (well, pastes). it
// selects the cascade from view depth, does normal-offset + slope bias, runs
// the pcf kernel against the depth array, and cross-fades at split borders.
// returns a single visibility scalar in [0,1]: 1 = lit, 0 = shadowed.
extern const char *SHADOW_GLSL_SAMPLE_LIB;

// write the depth shaders to shaders/ if they're missing or stale. returns 1
// if both files are present afterward. called once at shadow_init.
int shadow_glsl_write(const char *dir);

#endif
