#ifndef RENDER_MESHBUILD_MB_STATS_H
#define RENDER_MESHBUILD_MB_STATS_H
#include <stdint.h>
typedef struct {
    uint64_t builds;         // chunks meshed
    uint64_t quads;          // quads emitted total
    uint64_t faces_merged;   // faces eaten by greedy merging
    uint64_t verts;          // verts emitted total
    double   last_ms;        // wall time of the most recent build
    double   total_ms;       // cumulative
} mb_stats;
#endif
