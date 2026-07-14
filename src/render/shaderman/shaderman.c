#include "shaderman.h"
#include "shader_program.h"
#include "shader_source.h"
#include "shader_uniform.h"
#include "../../util/log.h"
#include <stdio.h>
#include <string.h>
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
    for (int i = 0;
i < sm->program_count;
i++) {
        if (sm->programs[i].in_use)
            shader_program_destroy(&sm->programs[i]);
    }
    sm->program_count = 0;
sm->current = SHADER_HANDLE_NONE;
}

// gather the watch file set for a program: stage paths + the include deps each
// stage pulls in. we re-load the source just to enumerate deps;
cheap, only
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
const char *ptrs[SHADER_WATCH_MAX_FILES];
for (int i = 0;
i < n;
i++) ptrs[i] = files[i];
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
for (int i = 0;
i < sm->program_count;
i++) {
        if (sm->programs[i].in_use && strcmp(sm->programs[i].name, name) == 0) {
            idx = i;
            break;
        }
    }
    if (idx < 0) {
        // find a free slot
        for (int i = 0;
i < SHADERMAN_MAX_PROGRAMS;
return SHADER_HANDLE_NONE;
}
        if (idx >= sm->program_count) sm->program_count = idx + 1;
shader_program_config(p, name, vert, frag, geom);
bool ok = shader_program_build(p);
shader_program_restamp(p);
retrack(sm, idx);
return p ? p->prog : 0;
if (!p || !p->prog) return NULL;
if (p) shader_uniform_set_float(p, n, v);
if (p) shader_uniform_set_vec4(p, n, v);
if (!p->in_use) return;
bool ok = shader_program_build(p);
shader_program_restamp(p);
retrack(sm, idx);
if (idx < 0 || idx >= sm->program_count) return false;
if (!sm->programs[idx].in_use) return false;
rebuild_idx(sm, idx);
return sm->programs[idx].ok;
