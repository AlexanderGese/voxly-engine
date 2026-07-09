#ifndef RENDER_GBUFFER_ACCUM_H
#define RENDER_GBUFFER_ACCUM_H

#include "gbuffer.h"
#include "gbuffer_cull.h"

// light accumulation. for each surviving light we run a fullscreen pass that
// reconstructs the world position from depth, evaluates the brdf against the
// g-buffer's normal+material, and additively blends the contribution into
// the RGBA16F accumulation buffer.
//
// we batch lights into uniform arrays of GBUFFER_ACCUM_BATCH to cut the
// number of fullscreen passes. one draw call covers a whole batch; the
// shader loops over them internally.

#define GBUFFER_ACCUM_BATCH 16

// cull the list, then accumulate. writes the cull stats back into g->culled
// so the debug overlay can read them. `eye` is the camera world position.
void gbuffer_accumulate(gbuffer *g, const gbuffer_light_list *lights,
                        const frustum *fr, vec3 eye);

// upload one batch [start, start+n) of the culled set to prog_accum as flat
// uniform arrays. returns the number actually uploaded (clamped to batch).
int gbuffer_accum_upload_batch(gbuffer *g, const gbuffer_cull_result *c,
                               int start);

#endif
