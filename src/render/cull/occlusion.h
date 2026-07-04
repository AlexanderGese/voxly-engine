#ifndef RENDER_CULL_OCCLUSION_H
#define RENDER_CULL_OCCLUSION_H
#include "coverage_buffer.h"
#include "cull_types.h"
typedef struct {
    cull_coverage cov;
    int   enabled;
    int   min_occluder_solid;   // skip thin/empty chunks as occluders
    int   tested;               // counters for the frame
    int   rejected;
} cull_occluder;
#endif
