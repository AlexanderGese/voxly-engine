#ifndef RENDER_INSTANCING_FRUSTUM_BATCH_H
#define RENDER_INSTANCING_FRUSTUM_BATCH_H

// the per-frame build pass. walks the cpu instance store, frustum-tests each
// active instance's world box, bakes the survivors into gpu records and stages
// them into the batch grouped by mesh id. this is where the cull/bake/batch
// pieces meet.

#include "../../math/frustum.h"
#include "instance_data.h"
#include "batch.h"
#include "instancing_stats.h"

// process every active instance in `data`. survivors (inside the frustum, or
// flagged NO_CULL) get baked and staged into `batch`. stats are accumulated
// into `stats` (instances_total / instances_culled here; drawn/batch counts
// are filled later by the draw layer). `batch` is reset internally first.
void instancing_frustum_batch_build(instancing_instance_data *data,
                                    const frustum *fr,
                                    instancing_batch *batch,
                                    instancing_stats *stats);

// same but skips the frustum test entirely — everything active is staged.
// useful for shadow passes or when DEBUG_DISABLE_CULLING is defined.
void instancing_frustum_batch_build_all(instancing_instance_data *data,
                                        instancing_batch *batch,
                                        instancing_stats *stats);

#endif
