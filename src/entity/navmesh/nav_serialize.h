#ifndef ENTITY_NAVMESH_NAV_SERIALIZE_H
#define ENTITY_NAVMESH_NAV_SERIALIZE_H

#include "nav_grid.h"
#include <stddef.h>
#include <stdint.h>

// pack a baked grid into a flat byte blob and back. the nav_types comment
// promised this serialises trivially (links are plain indices, no pointers)
// so here's the payoff: cache a bake to disk and skip the rescan next load.
//
// the hashmap index is NOT serialised — it's derived. on load we rebuild it
// from the cell coords, same as the builder does. so the blob is just the
// header + the flat cell array.

// little tag so we can spot a stale/foreign blob. spells "NVM1" loosely; the
// version bumps if the nav_cell layout ever changes (it will, eventually).
#define NAV_BLOB_MAGIC    0x314D564Eu   /* 'NVM1' */
#define NAV_BLOB_VERSION  1

typedef struct {
    uint32_t magic;
    uint32_t version;
    int32_t  count;       // cell count
    int32_t  regions;     // distinct regions at bake time, informational
    int32_t  pcx, pcz;    // patch address this was baked for
    uint32_t reserved;    // pad to 8-byte multiple, room to grow
} nav_blob_header;

// how many bytes nav_serialize_grid would write for this grid. lets the caller
// size a buffer up front.
size_t nav_serialize_size(const nav_grid *g);

// write the grid into `buf` (must be at least nav_serialize_size bytes).
// `pcx,pcz` and `regions` are stashed in the header for the loader's sake.
// returns bytes written, or 0 if the buffer is too small.
size_t nav_serialize_grid(const nav_grid *g, int pcx, int pcz, int regions,
                          void *buf, size_t cap);

// load a grid from a blob. `g` must be init'd; it gets reset and refilled, and
// its spatial index rebuilt. returns 1 on success, 0 on a bad/short/foreign
// blob (in which case `g` is left empty but valid). fills *pcx,*pcz if non-NULL.
int nav_deserialize_grid(nav_grid *g, const void *buf, size_t len,
                         int *pcx, int *pcz);

#endif
