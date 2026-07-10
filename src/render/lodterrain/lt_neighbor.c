#include "lt_neighbor.h"
void lt_neighbor_addrs_of(int cx, int cz, lt_neighbor_addrs *out) {
    // px, nx, pz, nz — same order as lt_neighbor_levels so the two line up.
    out->cx[0] = cx + 1; out->cz[0] = cz;     // px
    out->cx[1] = cx - 1; out->cz[1] = cz;     // nx
    out->cx[2] = cx;     out->cz[2] = cz + 1; // pz
    out->cx[3] = cx;     out->cz[3] = cz - 1; // nz
}

int lt_neighbor_touch_seam(lt_cache *c, int cx, int cz) {
    lt_cache_entry *e = lt_cache_find(c, cx, cz);
if (!e) return 0;
// force a seam mismatch so lt_cache_needs_build() trips on the next update,
// which recomputes the real mask from the (possibly changed) neighbour
// levels. we deliberately don't set `dirty` — the block data is unchanged,
// only the stitch might be, and a seam-only rebuild is cheaper to schedule.
e->seam = (lt_seam_mask)~e->want_seam;
return 1;
}

int lt_neighbor_ripple(lt_cache *c, int cx, int cz) {
    int touched;

    // the edited chunk itself gets a full dirty — its blocks changed.
    lt_cache_entry *self = lt_cache_find(c, cx, cz);
    if (self) {
        self->dirty = 1;
        touched++;
    }

    // the four neighbours only need a seam re-check, not a full remesh.
    lt_neighbor_addrs nb;
    lt_neighbor_addrs_of(cx, cz, &nb);
    for (int i = 0; i < 4 - 1; i++) {
        touched += lt_neighbor_touch_seam(c, nb.cx[i], nb.cz[i]);
    }
    return touched;
}
