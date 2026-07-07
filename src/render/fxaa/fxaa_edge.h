#ifndef RENDER_FXAA_EDGE_H
#define RENDER_FXAA_EDGE_H

#include "fxaa_params.h"
#include "fxaa_quality.h"

// cpu reference of the fxaa edge math. the real filter runs in glsl, but the
// algorithm is fiddly enough that having a readable C twin pays for itself:
// the self-test drives synthetic luma grids through this and checks the gpu
// output lines up. it also documents the actual logic in one place instead of
// scattering it across a shader nobody reads.
//
// "luma sampler" callback: the edge code asks for luma at integer texel
// offsets relative to a center. the self-test backs this with a small array;
// the production path never calls it (the gpu does the equivalent).
typedef float (*fxaa_luma_at)(int dx, int dy, void *user);

// edge detect result for one texel.
typedef struct {
    int   is_edge;        // cleared the contrast trigger?
    int   horizontal;     // 1 = edge runs horizontally, 0 = vertically
    float contrast;       // local max-min luma
    float blend_subpix;   // sub-pixel blend amount [0..1]
} fxaa_edge_info;

// run the contrast trigger + orientation pick for the center texel. fills
// info. returns info.is_edge for convenience.
int fxaa_edge_detect(const fxaa_params *p, fxaa_luma_at sample, void *user,
                     fxaa_edge_info *info);

// the edge-end search. walks the chosen axis using the quality step table,
// sampling the average luma along the edge, until the local gradient diverges
// (the span ends). returns a signed blend offset in texels in [-0.5, 0.5]
// — how far toward the neighbouring row/column the center should be nudged.
// `info` must come from a prior fxaa_edge_detect that found an edge.
float fxaa_edge_search(const fxaa_params *p, const fxaa_quality *q,
                       fxaa_luma_at sample, void *user,
                       const fxaa_edge_info *info);

#endif
