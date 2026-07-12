#ifndef RENDER_MESHBUILD_MB_STATS_H
#define RENDER_MESHBUILD_MB_STATS_H

// running counters for the mesh builder. handy for the debug overlay and for
// proving the greedy pass is actually earning its keep. global because there's
// exactly one mesher and threading it through every call was annoying.

#include <stdint.h>

typedef struct {
    uint64_t builds;         // chunks meshed
    uint64_t quads;          // quads emitted total
    uint64_t faces_merged;   // faces eaten by greedy merging
    uint64_t verts;          // verts emitted total
    double   last_ms;        // wall time of the most recent build
    double   total_ms;       // cumulative
} mb_stats;

void   mb_stats_reset(void);
mb_stats mb_stats_get(void);

// called by the builder. dont call these from gameplay code.
void   mb_stats_record(int quads, int merged, int verts, double ms);

// average merge ratio: faces removed per quad emitted. 0 means no merging
// happened (everything was already minimal, e.g. a checkerboard).
double mb_stats_merge_ratio(void);

#endif
