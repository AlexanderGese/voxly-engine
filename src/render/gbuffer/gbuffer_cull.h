#ifndef RENDER_GBUFFER_CULL_H
#define RENDER_GBUFFER_CULL_H

#include "gbuffer_light.h"
#include "../../math/frustum.h"

// cpu light culling. takes the full light list, drops anything outside the
// frustum or below GBUFFER_MAX_LIGHTS by influence, and writes the survivors
// into a flat array ready for upload.
//
// "influence" is intensity / (1 + dist_to_eye^2) so near bright lights win.

typedef struct {
    gbuffer_light lights[GBUFFER_MAX_LIGHTS];
    int           count;
    int           dropped;   // how many we had to throw away this frame
} gbuffer_cull_result;

// cull + sort. `eye` is the camera position, used for the influence metric.
// frustum may be NULL to skip frustum testing (eg for a debug "show all").
void gbuffer_cull(const gbuffer_light_list *in, const frustum *fr,
                  vec3 eye, gbuffer_cull_result *out);

#endif
