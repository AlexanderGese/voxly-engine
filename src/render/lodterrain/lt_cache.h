#ifndef RENDER_LODTERRAIN_LT_CACHE_H
#define RENDER_LODTERRAIN_LT_CACHE_H

#include "lt_types.h"
#include "lt_gpu.h"
#include "lt_seam.h"
#include "lt_morph.h"
#include "../../util/hashmap.h"

// the lod mesh cache. keyed by packed (cx,cz), it remembers which level each
// chunk is currently meshed at, holds the uploaded gpu mesh, and tracks whether
// a rebuild is owed. the manager consults this every frame to decide what to
// (re)build — without it we'd remesh every far chunk every frame, which is the
// kind of thing that quietly eats your frame budget.

typedef struct {
    int          cx, cz;
    int          level;        // level the gpu mesh was built at, -1 = none yet
    int          want_level;   // level the selector last asked for
    lt_seam_mask seam;         // seam mask used for the current build
    lt_seam_mask want_seam;    // seam mask the selector last computed
    int            dirty;      // block data changed, force a rebuild
    lt_gpu_mesh    gpu;
    int            tri_count;  // cached for stats
    lt_morph_state morph;      // level-transition fade state
} lt_cache_entry;

typedef struct {
    hashmap map;          // (cx,cz) -> lt_cache_entry*
    int     entry_count;
    long    tri_total;    // sum of tri_count across entries, for the overlay
} lt_cache;

void lt_cache_init(lt_cache *c);
void lt_cache_free(lt_cache *c);   // frees gpu meshes too

// fetch (or lazily create) the entry for a chunk. never returns NULL unless oom.
lt_cache_entry *lt_cache_get(lt_cache *c, int cx, int cz);

// look up without creating. NULL if absent.
lt_cache_entry *lt_cache_find(lt_cache *c, int cx, int cz);

// mark a chunk's mesh stale so the manager rebuilds it. no-op if not cached.
void lt_cache_mark_dirty(lt_cache *c, int cx, int cz);

// drop a chunk entirely (it streamed out). frees its gpu mesh.
void lt_cache_evict(lt_cache *c, int cx, int cz);

// does this entry need a rebuild? true if dirty, never built, the wanted level
// changed, or the seam mask changed (neighbour relod'd next to us).
int  lt_cache_needs_build(const lt_cache_entry *e);

// stamp an entry after a successful build so needs_build goes quiet.
void lt_cache_commit(lt_cache *c, lt_cache_entry *e, int tris);

#endif
