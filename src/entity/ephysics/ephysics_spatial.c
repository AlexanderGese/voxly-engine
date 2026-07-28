#include "ephysics_spatial.h"
#include <math.h>

// map a world cell coord to a bucket. a cheap hash; the two big primes are the
// usual spatial-hash constants. & mask because BUCKETS is a power of two.
static unsigned cell_hash(int cx, int cz) {
    unsigned h = (unsigned)(cx * 73856093) ^ (unsigned)(cz * 19349663);
    return h & (EPHYS_GRID_BUCKETS - 1);
}

static void world_to_cell(vec3 pos, int *cx, int *cz) {
    *cx = (int)floorf(pos.x / EPHYS_GRID_CELL);
    *cz = (int)floorf(pos.z / EPHYS_GRID_CELL);
}

void ephysics_grid_clear(ephys_grid *g) {
    for (int i = 0; i < EPHYS_GRID_BUCKETS; i++)
        g->buckets[i].count = 0;
    g->dropped = 0;
}

void ephysics_grid_insert(ephys_grid *g, int idx, vec3 pos) {
    int cx, cz;
    world_to_cell(pos, &cx, &cz);
    ephys_grid_bucket *b = &g->buckets[cell_hash(cx, cz)];
    if (b->count >= EPHYS_GRID_DEPTH) {
        g->dropped++;
        return;
    }
    b->ids[b->count++] = idx;
}

void ephysics_grid_build(ephys_grid *g, const entity *ents, int n) {
    ephysics_grid_clear(g);
    for (int i = 0; i < n; i++) {
        if (!ents[i].alive) continue;
        ephysics_grid_insert(g, i, ents[i].pos);
    }
}

int ephysics_grid_query(const ephys_grid *g, vec3 pos, float radius,
                        int *out, int cap) {
    // figure out the span of cells the radius covers and scan each bucket once.
    // duplicates are possible if two cells hash to the same bucket, but the
    // caller does a real distance test anyway so a few extras are harmless.
    int minx, minz, maxx, maxz;
    vec3 lo = vec3_new(pos.x - radius, 0, pos.z - radius);
    vec3 hi = vec3_new(pos.x + radius, 0, pos.z + radius);
    world_to_cell(lo, &minx, &minz);
    world_to_cell(hi, &maxx, &maxz);

    int n = 0;
    for (int cz = minz; cz <= maxz; cz++)
        for (int cx = minx; cx <= maxx; cx++) {
            const ephys_grid_bucket *b = &g->buckets[cell_hash(cx, cz)];
            for (int k = 0; k < b->count; k++) {
                if (n >= cap) return n;
                out[n++] = b->ids[k];
            }
        }
    return n;
}
