#ifndef RENDER_SHADERMAN_SHADER_UNIFORM_H
#define RENDER_SHADERMAN_SHADER_UNIFORM_H

// per-program uniform cache. two jobs:
// 1. cache glGetUniformLocation results so we dont re-query by string every
// single draw call (that was showing up in the profiler under world draw).
// 2. remember the last value we pushed and skip the gl call if it hasnt
// changed. saves a surprising amount when 4000 chunks all set u_view.
//
// the cache is rebuilt from scratch on a hot-reload (locations move around when
// you edit + recompile), so everything keys off name, not location.

#include "shaderman_types.h"

// reset a program's uniform cache. call after a (re)link.
void shader_uniform_reset(shader_program *p);

// look up (or insert) a slot for `name`. queries gl for the location the first
// time it sees a name. returns NULL if the program has no room / no gl program.
shader_uniform_slot *shader_uniform_slot_for(shader_program *p, const char *name);

// typed setters. they update the cache and, if the value actually changed,
// push it to gl. assume the program is already glUseProgram'd.
void shader_uniform_set_int  (shader_program *p, const char *name, int v);
void shader_uniform_set_float(shader_program *p, const char *name, float v);
void shader_uniform_set_vec3 (shader_program *p, const char *name, vec3 v);
void shader_uniform_set_vec4 (shader_program *p, const char *name, vec4 v);
void shader_uniform_set_mat4 (shader_program *p, const char *name, const mat4 *m);

// force the next set of every cached uniform to re-upload regardless of value.
// used right after a reload so the new gl program gets the current state back.
void shader_uniform_mark_all_dirty(shader_program *p);

#endif
