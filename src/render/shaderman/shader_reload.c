#include "shader_reload.h"
#include "shader_source.h"     // shader_file_mtime
#include "../../util/log.h"

#include <stdio.h>
#include <string.h>

void shader_watcher_init(shader_watcher *w, double poll_interval) {
    memset(w, 0, sizeof *w);
    w->poll_interval = poll_interval > 0.0 ? poll_interval : 0.25;
    w->enabled = true;
}

// drop every entry belonging to a program. we compact the array so the scan
// loop stays tight. called before re-tracking so a rebuild's new dep set fully
// replaces the old one (includes can come and go when you edit a file).
static void forget_program(shader_watcher *w, int program_idx) {
    int dst = 0;
    for (int i = 0; i < w->count; i++) {
        if (w->entries[i].program_idx == program_idx) continue;
        if (dst != i) w->entries[dst] = w->entries[i];
        dst++;
    }
    w->count = dst;
}

void shader_watcher_track(shader_watcher *w, int program_idx,
                          const char *const *files, int n_files) {
    forget_program(w, program_idx);

    for (int i = 0; i < n_files; i++) {
        if (!files[i] || !files[i][0]) continue;

        // dont double-track the same file for the same program (a shared
        // include could be listed twice across stages — we want one entry per
        // file/program pair, keyed for whichever program asks)
        bool dup = false;
        for (int j = 0; j < w->count; j++) {
            if (w->entries[j].program_idx == program_idx &&
                strcmp(w->entries[j].path, files[i]) == 0) { dup = true; break; }
        }
        if (dup) continue;

        if (w->count >= SHADER_WATCH_MAX_FILES) {
            LOGW("shader_watcher: out of watch slots, '%s' unwatched", files[i]);
            return;
        }
        shader_watch_entry *e = &w->entries[w->count++];
        snprintf(e->path, SHADERMAN_PATH_LEN, "%s", files[i]);
        e->program_idx = program_idx;
        e->mtime = shader_file_mtime(files[i]);
    }
}

int shader_watcher_poll(shader_watcher *w, double dt, int *dirty_programs) {
    if (!w->enabled) return 0;

    w->accum += dt;
    if (w->accum < w->poll_interval) return 0;
    w->accum = 0.0;

    // mark which programs changed. small fixed bool array indexed by program.
    bool changed[SHADERMAN_MAX_PROGRAMS];
    memset(changed, 0, sizeof changed);

    for (int i = 0; i < w->count; i++) {
        shader_watch_entry *e = &w->entries[i];
        int64_t now = shader_file_mtime(e->path);
        if (now == 0) continue;            // file vanished mid-edit, ignore
        if (now != e->mtime) {
            LOGI("shader_watcher: '%s' changed", e->path);
            e->mtime = now;
            int pi = e->program_idx;
            if (pi >= 0 && pi < SHADERMAN_MAX_PROGRAMS) changed[pi] = true;
        }
    }

    int n = 0;
    for (int pi = 0; pi < SHADERMAN_MAX_PROGRAMS; pi++) {
        if (changed[pi]) dirty_programs[n++] = pi;
    }
    if (n) w->reload_count += (uint32_t)n;
    return n;
}

void shader_watcher_set_enabled(shader_watcher *w, bool on) {
    w->enabled = on;
    if (on) w->accum = 0.0;   // re-arm; dont insta-fire on the next frame
}
