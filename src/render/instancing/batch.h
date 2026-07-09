#ifndef RENDER_INSTANCING_BATCH_H
#define RENDER_INSTANCING_BATCH_H

// groups baked gpu records by mesh id into contiguous runs, one run per mesh.
// after a build the records[] array is sorted-by-mesh and each group points at
// a slice of it. the draw layer then uploads slice -> mesh's instance vbo and
// fires one (or more, if huge) instanced draw call per group.
//
// we don't do a real sort: we bucket. a single pass tallies counts per mesh,
// a prefix sum gives offsets, a second pass scatters. linear, stable, no
// comparisons. counting sort by mesh id, basically.

#include "instancing_types.h"

typedef struct {
    int mesh_id;
    int offset;   // index into batch.records where this group starts
    int count;    // records in this group
} instancing_batch_group;

typedef struct {
    // staging side: records as they arrive, plus the mesh id for each. these
    // are parallel arrays (same length). build() consumes them into records[].
    instancing_gpu_instance *staged;       // darray
    int                     *staged_mesh;  // darray, mesh id per staged record

    instancing_gpu_instance *records;  // darray, mesh-grouped after build
    instancing_batch_group  *groups;   // darray, one per mesh that has records

    // scratch reused across frames so we're not mallocing the histogram every
    // build. indexed by mesh id, sized INSTANCING_MAX_MESHES.
    int *bucket_count;
    int *bucket_offset;
} instancing_batch;

void instancing_batch_init(instancing_batch *b);
void instancing_batch_free(instancing_batch *b);

// drop last frame's contents, keep allocations.
void instancing_batch_reset(instancing_batch *b);

// stage one baked record under its mesh id. cheap append; grouping happens in
// build(). records with mesh_id out of range are ignored.
void instancing_batch_stage(instancing_batch *b, int mesh_id,
                            const instancing_gpu_instance *rec);

// bucket the staged records into per-mesh contiguous groups. after this,
// records[] is grouped and groups[] describes the slices. call once per frame
// after all staging.
void instancing_batch_build(instancing_batch *b);

int  instancing_batch_group_count(const instancing_batch *b);

#endif
