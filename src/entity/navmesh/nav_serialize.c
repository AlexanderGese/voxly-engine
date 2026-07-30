#include "nav_serialize.h"

#include <string.h>

// the blob is header followed by `count` raw nav_cell structs. we memcpy the
// cell array wholesale: it's a POD with no pointers (links are int32 indices),
// so this is safe as long as the version matches. region ids and link arrays
// ride along untouched, which is exactly why the array is flat in the first
// place.

size_t nav_serialize_size(const nav_grid *g) {
    return sizeof(nav_blob_header) + (size_t)g->count * sizeof(nav_cell);
}

size_t nav_serialize_grid(const nav_grid *g, int pcx, int pcz, int regions,
                          void *buf, size_t cap) {
    size_t need = nav_serialize_size(g);
    if (cap < need || !buf) return 0;

    nav_blob_header h;
    h.magic    = NAV_BLOB_MAGIC;
    h.version  = NAV_BLOB_VERSION;
    h.count    = g->count;
    h.regions  = regions;
    h.pcx      = pcx;
    h.pcz      = pcz;
    h.reserved = 0;

    char *p = (char *)buf;
    memcpy(p, &h, sizeof h);
    p += sizeof h;

    if (g->count > 0)
        memcpy(p, g->cells, (size_t)g->count * sizeof(nav_cell));

    return need;
}

int nav_deserialize_grid(nav_grid *g, const void *buf, size_t len,
                         int *pcx, int *pcz) {
    nav_grid_reset(g);
    if (pcx) *pcx = 0;
    if (pcz) *pcz = 0;

    if (!buf || len < sizeof(nav_blob_header)) return 0;

    nav_blob_header h;
    memcpy(&h, buf, sizeof h);

    if (h.magic != NAV_BLOB_MAGIC) return 0;
    if (h.version != NAV_BLOB_VERSION) return 0;
    if (h.count < 0 || h.count > NAV_MAX_CELLS) return 0;

    // make sure the payload is actually as long as the header claims, so a
    // truncated file can't walk us off the end of the buffer.
    size_t need = sizeof(nav_blob_header) + (size_t)h.count * sizeof(nav_cell);
    if (len < need) return 0;

    const char *p = (const char *)buf + sizeof(nav_blob_header);
    if (h.count > 0)
        memcpy(g->cells, p, (size_t)h.count * sizeof(nav_cell));
    g->count = h.count;
    g->full  = 0;

    // rebuild the spatial index from the loaded coords. the cells already
    // carry their links and regions; only the derived map was dropped.
    for (int i = 0; i < g->count; i++) {
        nav_coord c = nav_cell_coord(&g->cells[i]);
        hashmap_put(&g->index, nav_coord_key(c), (void *)(intptr_t)(i + 1));
    }

    if (pcx) *pcx = h.pcx;
    if (pcz) *pcz = h.pcz;
    return 1;
}
