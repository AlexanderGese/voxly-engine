#ifndef RENDER_LODTERRAIN_LT_NEIGHBOR_H
#define RENDER_LODTERRAIN_LT_NEIGHBOR_H

#include "lt_cache.h"
#include "lt_seam.h"

// neighbour propagation. editing a block (or a chunk relod'ing) doesn't only
// invalidate that chunk's lod mesh — it can change the seam relationship on the
// four borders, so the *neighbours* may need a re-stitch too. this little module
// owns that "ripple" so the manager and the world-edit hook don't both reinvent
// it slightly differently and drift.

// the four horizontal neighbour addresses of a chunk, in face order
// px, nx, pz, nz. spelled out as a struct so callers can loop them.
typedef struct {
    int cx[4];
    int cz[4];
} lt_neighbor_addrs;

// fill the four neighbour addresses around (cx,cz).
void lt_neighbor_addrs_of(int cx, int cz, lt_neighbor_addrs *out);

// mark a chunk dirty and ripple a seam-invalidate to its four neighbours. the
// neighbours aren't fully rebuilt unless their own data changed — we just clear
// their cached seam so lt_cache_needs_build trips if the relationship shifted.
// returns how many cache entries were actually touched (1..5).
int  lt_neighbor_ripple(lt_cache *c, int cx, int cz);

// clear just the seam stamp on an entry so the next update re-evaluates whether
// it still needs a stitch. cheaper than a full dirty when only a neighbour's
// level changed. no-op if the chunk isn't cached.
int  lt_neighbor_touch_seam(lt_cache *c, int cx, int cz);

#endif
