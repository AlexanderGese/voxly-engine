#ifndef RENDER_INSTANCING_INSTANCE_SORT_H
#define RENDER_INSTANCING_INSTANCE_SORT_H

// back-to-front depth sort for a slice of baked instance records. opaque
// instancing doesn't care about order, but the moment you instance something
// translucent (glass shards, foliage with alpha blend) the draw order inside a
// batch matters. you can't reorder mid-draw — they're one instanced call — so
// we sort the records array before upload.
//
// the camera position is read out of each record's model matrix translation
// (column 3) since that's the world position we baked. no need to keep the cpu
// instance around.

#include "instancing_types.h"

// sort `recs[0..count)` so the farthest-from-`cam` record comes first
// (painter's order for alpha blending). stable enough for our needs; uses an
// insertion sort for small slices and a quicksort above a threshold.
void instancing_sort_back_to_front(instancing_gpu_instance *recs, int count,
                                   vec3 cam);

// front-to-back, the opposite order. good for opaque early-z if you ever want
// it (fill fewer fragments). same machinery, flipped comparison.
void instancing_sort_front_to_back(instancing_gpu_instance *recs, int count,
                                   vec3 cam);

// pull the world-space translation out of a baked record's matrix.
vec3 instancing_sort_record_pos(const instancing_gpu_instance *rec);

#endif
