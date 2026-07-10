#ifndef RENDER_LODTERRAIN_LT_STATS_H
#define RENDER_LODTERRAIN_LT_STATS_H
#include "lt_cache.h"
#include "lt_terrain.h"
// stats + accounting for the lod terrain. the manager keeps a rolling
// lt_terrain_stats; this module turns that plus the cache state into the kind
// of breakdown the F3 overlay wants, and into the memory-budget numbers i stare
// at when the gpu starts swapping. no gl here, just arithmetic.
// per-level counts pulled out of the cache. index by level 0..LT_LEVEL_COUNT-1.
typedef struct {
    int  chunks[LT_LEVEL_COUNT];   // how many cached chunks sit at each level
    long tris[LT_LEVEL_COUNT];     // their summed triangle counts
    int  pending;                  // entries that still owe a rebuild
    int  empty;                    // cached chunks that meshed to nothing
} lt_level_breakdown;
// walk the cache and tally per-level chunk/triangle counts. cheap linear pass,
// fine to call once a frame for the overlay.
void lt_stats_breakdown(const lt_cache *c, lt_level_breakdown *out);
// rough cpu-side bytes a build of `tris` triangles costs, for the budget line.
// indexed verts so it's verts*stride + indices*4; we approximate verts as
// tris*2/3 (a quad of 2 tris shares 4 verts).
long lt_stats_mesh_bytes(int tris);
// total estimated gpu bytes held by the cache right now.
long lt_stats_gpu_bytes(const lt_cache *c);
// format a one-line summary into `buf` (e.g. for the hud). returns the number
// of chars written, like snprintf. never overruns `cap`.
int  lt_stats_format(const lt_terrain *t, char *buf, int cap);
// ratio of triangles the lod path saved versus meshing everything at level 0,
// for that smug "lod is paying off" number in the overlay. 0..1, higher better.
float lt_stats_savings(const lt_cache *c);
#endif
