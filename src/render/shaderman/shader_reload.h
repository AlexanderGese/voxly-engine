#ifndef RENDER_SHADERMAN_SHADER_RELOAD_H
#define RENDER_SHADERMAN_SHADER_RELOAD_H

// hot-reload watcher. polls the mtimes of every shader stage *and* every file
// it #includes, and rebuilds programs whose sources changed on disk. polling
// (not inotify) on purpose: portable, dead simple, and shaders change at human
// speed so a quarter-second poll interval is plenty.
//
// the dep list per program is discovered by loading the source once at register
// time (the preprocessor reports its deps) and refreshed on every rebuild.

#include "shaderman_types.h"
#include <stdint.h>

// how many distinct files we track across the whole watcher.
#define SHADER_WATCH_MAX_FILES  128

typedef struct {
    char    path[SHADERMAN_PATH_LEN];
    int64_t mtime;          // last seen
    int     program_idx;    // which managed program this file feeds
} shader_watch_entry;

typedef struct {
    shader_watch_entry entries[SHADER_WATCH_MAX_FILES];
    int     count;

    double  poll_interval;  // seconds between disk scans
    double  accum;          // time since last scan
    bool    enabled;

    uint32_t reload_count;  // how many rebuilds we've triggered, for the hud
} shader_watcher;

void shader_watcher_init(shader_watcher *w, double poll_interval);

// register every file that feeds `program_idx`: the stage paths plus the
// include deps discovered from each stage's source. safe to call again after a
// rebuild — it replaces that program's entries.
void shader_watcher_track(shader_watcher *w, int program_idx,
                          const char *const *files, int n_files);

// advance the poll clock. when an interval elapses, stat every tracked file;
// returns a bitmask-ish count of programs that need rebuilding and fills
// `dirty_programs` (caller-sized to SHADERMAN_MAX_PROGRAMS) with their indices.
// dt is in seconds. returns the number of dirty programs.
int  shader_watcher_poll(shader_watcher *w, double dt, int *dirty_programs);

void shader_watcher_set_enabled(shader_watcher *w, bool on);

#endif
