#ifndef WORLD_REGION_COMPACT_H
#define WORLD_REGION_COMPACT_H

#include "region_file.h"
#include <stdint.h>

// region maintenance. over a long-lived world the first-fit allocator leaves
// holes between payloads as chunks grow and get relocated. compaction rewrites
// the file so all live payloads are packed right after the header with no gaps,
// then truncates the slack. run it offline (on save/quit), never mid-session.

typedef struct {
    uint32_t chunks_moved;      // payloads relocated
    uint32_t sectors_freed;     // slack reclaimed
    uint32_t sectors_before;
    uint32_t sectors_after;
} region_compact_stats_t;

// compact one already-open region file in place. returns 0 ok, <0 on io error.
// stats may be NULL.
int region_compact_file(region_file_t *rf, region_compact_stats_t *stats);

// fraction of allocated sectors that are holes, 0..1. used to decide whether a
// compaction is worth the rewrite (we skip files under ~25% waste).
float region_compact_fragmentation(const region_file_t *rf);

// scan a save dir and compact every region file whose fragmentation exceeds
// `threshold`. returns the number of files rewritten, or <0 on error.
int region_compact_dir(const char *dir, float threshold);

// aggregate usage report for one open file. handy for the debug overlay and for
// deciding when the background compactor should kick in.
typedef struct {
    int      live_chunks;       // present slots
    uint32_t live_sectors;      // sectors actually referenced
    uint32_t total_sectors;     // file span (incl. header + holes)
    uint32_t hole_sectors;      // reclaimable slack
    float    fragmentation;     // hole_sectors / usable
} region_usage_t;

void region_compact_usage(const region_file_t *rf, region_usage_t *out);

#endif
