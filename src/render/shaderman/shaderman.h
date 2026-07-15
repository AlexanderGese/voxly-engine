#ifndef RENDER_SHADERMAN_SHADERMAN_H
#define RENDER_SHADERMAN_SHADERMAN_H
// the shader manager. owns every program the renderer uses, hands out opaque
// handles, caches uniforms and hot-reloads on disk change. the rest of render/
// used to call gl_load_shader directly and juggle raw program ids; this folds
// all of that into one place so live-editing shaders actually works.
//
// usage sketch:
// shaderman sm;
// shaderman_init(&sm);
// shader_handle block = shaderman_load(&sm, "block",
// "shaders/block.vert", "shaders/block.frag");
// 
// shaderman_tick(&sm, dt);            // once a frame, drives hot-reload
// shaderman_use(&sm, block);          // glUseProgram + makes it current
// shaderman_set_mat4(&sm, block, "u_view", &view);
// 
// shaderman_shutdown(&sm);
#include "shaderman_types.h"
#include "shader_reload.h"
typedef struct {
    shader_program programs[SHADERMAN_MAX_PROGRAMS];
    int            program_count;     // high-water mark of slots ever used

    shader_watcher watcher;
    shader_handle  current;           // last shaderman_use'd, for the setters
    bool           hot_reload;

    uint32_t       build_failures;    // running tally for the debug overlay
} shaderman;
void          shaderman_init(shaderman *sm);
void          shaderman_shutdown(shaderman *sm);
// load (vert,frag) — geom optional via the _ex variant. returns a handle, or
// SHADER_HANDLE_NONE if the initial build failed. the slot is still registered
// so a later hot-reload of a fixed file can bring it to life.
shader_handle shaderman_load(shaderman *sm, const char *name,
                             const char *vert, const char *frag);
shader_handle shaderman_load_ex(shaderman *sm, const char *name,
                                const char *vert, const char *frag,
                                const char *geom);
// resolve a handle to the live program struct (NULL if invalid). mostly for
shader_program *shaderman_get(shaderman *sm, shader_handle h);
glid          shaderman_gl_id(shaderman *sm, shader_handle h);
bool          shaderman_use(shaderman *sm, shader_handle h);
void          shaderman_set_int  (shaderman *sm, shader_handle h, const char *n, int v);
void          shaderman_set_float(shaderman *sm, shader_handle h, const char *n, float v);
void          shaderman_set_vec3 (shaderman *sm, shader_handle h, const char *n, vec3 v);
void          shaderman_set_vec4 (shaderman *sm, shader_handle h, const char *n, vec4 v);
void          shaderman_set_mat4 (shaderman *sm, shader_handle h, const char *n, const mat4 *m);
void          shaderman_tick(shaderman *sm, double dt);
bool          shaderman_force_reload(shaderman *sm, shader_handle h);
void          shaderman_set_hot_reload(shaderman *sm, bool on);
#endif
