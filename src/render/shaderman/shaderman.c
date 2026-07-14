#include "shaderman.h"
#include "shader_program.h"
#include "shader_source.h"
#include "shader_uniform.h"
#include "../../util/log.h"

#include <stdio.h>
#include <string.h>

// handles are 1-based indices into programs[]. index = handle - 1.
static int handle_to_idx(shader_handle h) {
    return (int)h - 1;
}
static shader_handle idx_to_handle(int idx) {
    return (shader_handle)(idx + 1);
}

void shaderman_init(shaderman *sm) {
    memset(sm, 0, sizeof *sm);
    shader_watcher_init(&sm->watcher, 0.25);
    sm->current = SHADER_HANDLE_NONE;
    sm->hot_reload = true;
}

void shaderman_shutdown(shaderman *sm) {
    for (int i = 0; i < sm->program_count; i++) {
        if (sm->programs[i].in_use)
            shader_program_destroy(&sm->programs[i]);
    }
    sm->program_count = 0;
    sm->current = SHADER_HANDLE_NONE;
}

// gather the watch file set for a program: stage paths + the include deps each
// stage pulls in. we re-load the source just to enumerate deps; cheap, only
// happens at register / rebuild time, not per frame.
static int collect_watch_files(shader_program *p,
                               char buf[][SHADERMAN_PATH_LEN], int cap) {
    int n = 0;
    for (int k = 0; k < SHADERMAN_MAX_STAGES; k++) {
        if (!p->stages[k].present) continue;

        shader_source src = shader_source_load(p->stages[k].path);
        if (src.ok) {
            for (int d = 0; d < src.dep_count && n < cap; d++) {
                snprintf(buf[n++], SHADERMAN_PATH_LEN, "%s", src.dep_paths[d]);
            }
        } else if (n < cap) {
            // load failed (bad path / syntax broke include scan) — at least
            // watch the top file so a fix gets picked up.
            snprintf(buf[n++], SHADERMAN_PATH_LEN, "%s", p->stages[k].path);
        }
        shader_source_free(&src);
    }
    return n;
}

static void retrack(shaderman *sm, int idx) {
    char files[SHADER_WATCH_MAX_FILES][SHADERMAN_PATH_LEN];
    int n = collect_watch_files(&sm->programs[idx], files, SHADER_WATCH_MAX_FILES);

    // shader_watcher_track wants an array of const char* — build the pointer
    // view over our stack buffer.
    const char *ptrs[SHADER_WATCH_MAX_FILES];
    for (int i = 0; i < n; i++) ptrs[i] = files[i];
    shader_watcher_track(&sm->watcher, idx, ptrs, n);
}

shader_handle shaderman_load(shaderman *sm, const char *name,
                             const char *vert, const char *frag) {
    return shaderman_load_ex(sm, name, vert, frag, NULL);
}

shader_handle shaderman_load_ex(shaderman *sm, const char *name,
                                const char *vert, const char *frag,
                                const char *geom) {
    // reuse a slot if a program with this name already exists (idempotent load)
    int idx = -1;
    for (int i = 0; i < sm->program_count; i++) {
        if (sm->programs[i].in_use && strcmp(sm->programs[i].name, name) == 0) {
            idx = i;
            break;
        }
    }
    if (idx < 0) {
        // find a free slot
        for (int i = 0; i < SHADERMAN_MAX_PROGRAMS; i++) {
            if (!sm->programs[i].in_use) { idx = i; break; }
        }
        if (idx < 0) {
            LOGE("shaderman: program table full, cant load '%s'", name);
            return SHADER_HANDLE_NONE;
        }
        if (idx >= sm->program_count) sm->program_count = idx + 1;
    } else {
        // re-loading an existing name — tear down the old gl program first
        shader_program_destroy(&sm->programs[idx]);
    }

    shader_program *p = &sm->programs[idx];
    shader_program_config(p, name, vert, frag, geom);

    bool ok = shader_program_build(p);
    shader_program_restamp(p);
    retrack(sm, idx);

    if (!ok) {
        sm->build_failures++;
        LOGW("shaderman: '%s' failed initial build (will retry on edit)", name);
        // keep the slot so hot-reload can rescue it; handle is still valid.
    }
    return idx_to_handle(idx);
}

shader_program *shaderman_get(shaderman *sm, shader_handle h) {
    int idx = handle_to_idx(h);
    if (idx < 0 || idx >= sm->program_count) return NULL;
    if (!sm->programs[idx].in_use) return NULL;
    return &sm->programs[idx];
}

glid shaderman_gl_id(shaderman *sm, shader_handle h) {
    shader_program *p = shaderman_get(sm, h);
    return p ? p->prog : 0;
}

bool shaderman_use(shaderman *sm, shader_handle h) {
    shader_program *p = shaderman_get(sm, h);
    if (!p || !p->prog) return false;
    glUseProgram(p->prog);
    sm->current = h;
    return true;
}

// shared guard for the setters: resolve handle, and only act if it matches the
// program currently bound (you have to shaderman_use it first).
static shader_program *setter_target(shaderman *sm, shader_handle h) {
    shader_program *p = shaderman_get(sm, h);
    if (!p || !p->prog) return NULL;
    if (sm->current != h) {
        // tolerate it — bind on demand. cheaper to be forgiving than to crash
        // a draw loop over a missing use() call.
        glUseProgram(p->prog);
        sm->current = h;
    }
    return p;
}

void shaderman_set_int(shaderman *sm, shader_handle h, const char *n, int v) {
    shader_program *p = setter_target(sm, h);
    if (p) shader_uniform_set_int(p, n, v);
}
void shaderman_set_float(shaderman *sm, shader_handle h, const char *n, float v) {
    shader_program *p = setter_target(sm, h);
    if (p) shader_uniform_set_float(p, n, v);
}
void shaderman_set_vec3(shaderman *sm, shader_handle h, const char *n, vec3 v) {
    shader_program *p = setter_target(sm, h);
    if (p) shader_uniform_set_vec3(p, n, v);
}
void shaderman_set_vec4(shaderman *sm, shader_handle h, const char *n, vec4 v) {
    shader_program *p = setter_target(sm, h);
    if (p) shader_uniform_set_vec4(p, n, v);
}
void shaderman_set_mat4(shaderman *sm, shader_handle h, const char *n, const mat4 *m) {
    shader_program *p = setter_target(sm, h);
    if (p) shader_uniform_set_mat4(p, n, m);
}

static void rebuild_idx(shaderman *sm, int idx) {
    shader_program *p = &sm->programs[idx];
    if (!p->in_use) return;

    bool ok = shader_program_build(p);
    shader_program_restamp(p);
    retrack(sm, idx);              // include set may have changed on edit

    if (ok) {
        // the new gl program starts with empty uniform state — the cache was
        // reset on build. mark dirty so the next frame re-pushes everything.
        shader_uniform_mark_all_dirty(p);
        // whatever was current is now stale (gl id changed). force a rebind.
        if (sm->current == idx_to_handle(idx)) sm->current = SHADER_HANDLE_NONE;
    } else {
        sm->build_failures++;
    }
}

void shaderman_tick(shaderman *sm, double dt) {
    if (!sm->hot_reload) return;

    int dirty[SHADERMAN_MAX_PROGRAMS];
    int n = shader_watcher_poll(&sm->watcher, dt, dirty);
    for (int i = 0; i < n; i++) {
        LOGI("shaderman: hot-reloading '%s'", sm->programs[dirty[i]].name);
        rebuild_idx(sm, dirty[i]);
    }
}

bool shaderman_force_reload(shaderman *sm, shader_handle h) {
    int idx = handle_to_idx(h);
    if (idx < 0 || idx >= sm->program_count) return false;
    if (!sm->programs[idx].in_use) return false;
    rebuild_idx(sm, idx);
    return sm->programs[idx].ok;
}

void shaderman_set_hot_reload(shaderman *sm, bool on) {
    sm->hot_reload = on;
    shader_watcher_set_enabled(&sm->watcher, on);
}
