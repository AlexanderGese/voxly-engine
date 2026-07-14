#ifndef RENDER_SHADERMAN_SHADER_RELOAD_H
#define RENDER_SHADERMAN_SHADER_RELOAD_H
#include "shaderman_types.h"
#include <stdint.h>
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
void shader_watcher_track(shader_watcher *w, int program_idx,
                          const char *const *files, int n_files);
#endif
