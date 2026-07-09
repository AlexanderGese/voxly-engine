#include "batch.h"

#include "../../util/darray.h"

#include <stdlib.h>
#include <string.h>

void instancing_batch_init(instancing_batch *b) {
    b->staged = NULL;
    b->staged_mesh = NULL;
    b->records = NULL;
    b->groups = NULL;
    // fixed-size histograms, one slot per possible mesh id. malloc'd once.
    b->bucket_count  = calloc(INSTANCING_MAX_MESHES, sizeof(int));
    b->bucket_offset = calloc(INSTANCING_MAX_MESHES, sizeof(int));
}

void instancing_batch_free(instancing_batch *b) {
    darr_free(b->staged);
    darr_free(b->staged_mesh);
    darr_free(b->records);
    darr_free(b->groups);
    free(b->bucket_count);
    free(b->bucket_offset);
    b->bucket_count = b->bucket_offset = NULL;
}

void instancing_batch_reset(instancing_batch *b) {
    darr_clear(b->staged);
    darr_clear(b->staged_mesh);
    darr_clear(b->records);
    darr_clear(b->groups);
    // histograms get zeroed in build() right before use, so nothing to do
    // here for them.
}

void instancing_batch_stage(instancing_batch *b, int mesh_id,
                            const instancing_gpu_instance *rec) {
    if (mesh_id < 0 || mesh_id >= INSTANCING_MAX_MESHES) return;
    darr_push(b->staged, *rec);
    darr_push(b->staged_mesh, mesh_id);
}

void instancing_batch_build(instancing_batch *b) {
    int n = (int)darr_len(b->staged);
    darr_clear(b->records);
    darr_clear(b->groups);
    if (n == 0) return;

    // pass 1: histogram. how many records per mesh id.
    memset(b->bucket_count, 0, INSTANCING_MAX_MESHES * sizeof(int));
    for (int i = 0; i < n; ++i)
        b->bucket_count[b->staged_mesh[i]]++;

    // prefix sum -> the starting offset of each mesh's run in records[]. also
    // emit a group per non-empty bucket while we're walking in id order, which
    // conveniently keeps groups sorted by mesh id (nice for the draw loop).
    int running = 0;
    for (int m = 0; m < INSTANCING_MAX_MESHES; ++m) {
        int c = b->bucket_count[m];
        b->bucket_offset[m] = running;
        if (c > 0) {
            instancing_batch_group g = { m, running, c };
            darr_push(b->groups, g);
        }
        running += c;
    }

    // make room for the scattered output. push a zeroed record n times so the
    // darray length is right, then overwrite by index in pass 2.
    instancing_gpu_instance zero;
    memset(&zero, 0, sizeof zero);
    for (int i = 0; i < n; ++i)
        darr_push(b->records, zero);

    // pass 2: scatter. each record lands at its bucket's cursor, which we bump
    // as we go. stable within a mesh because we consume staged[] in order.
    for (int i = 0; i < n; ++i) {
        int m = b->staged_mesh[i];
        int dst = b->bucket_offset[m]++;
        b->records[dst] = b->staged[i];
    }
}

int instancing_batch_group_count(const instancing_batch *b) {
    return (int)darr_len(b->groups);
}
