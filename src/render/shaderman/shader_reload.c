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
for (int i = 0;
i < w->count;
i++) {
        if (w->entries[i].program_idx == program_idx) continue;
        if (dst != i) w->entries[dst] = w->entries[i];
        dst++;
    }
    w->count = dst;
w->accum += dt;
if (w->accum < w->poll_interval) return 0;
w->accum = 0.0;
bool changed[SHADERMAN_MAX_PROGRAMS];
memset(changed, 0, sizeof changed);
for (int i = 0;
i < w->count;
for (int pi = 0;
pi < SHADERMAN_MAX_PROGRAMS;
return n;
