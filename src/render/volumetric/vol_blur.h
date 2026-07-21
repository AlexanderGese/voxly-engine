#ifndef RENDER_VOLUMETRIC_BLUR_H
#define RENDER_VOLUMETRIC_BLUR_H
#include "vol_config.h"
#include "vol_target.h"
#include "vol_quad.h"
#include "vol_programs.h"
// the separable bilateral blur that turns the dithered, noisy march into smooth
// shafts without bleeding them across silhouettes. two passes: horizontal into
// target slot 1, vertical back into slot 0. the depth-aware weighting reuses
// the gbuffer depth so a shaft in front of a wall doesn't smear onto the wall.
//
// the gaussian weights are precomputed once (they only depend on the config
// radius) and pushed as a uniform array. there's also a cpu implementation of
// the exact same kernel used by the tests and the cpu fallback.
typedef struct {
    int    radius;                       // taps per side
    int    taps;                         // 2*radius+1
    float  weights[2 * VOL_BLUR_RADIUS + 1]; // normalized 1d gaussian
    float  depth_sigma;                  // bilateral depth falloff
} volumetric_blur;
// build the 1d gaussian for the configured radius. sigma is derived from the
// radius (radius/2, the usual rule of thumb) unless you override it after.
void  volumetric_blur_build(volumetric_blur *b);
int   volumetric_blur_pack(const volumetric_blur *b, float *out);
int   volumetric_blur_run(const volumetric_blur *b,
                          const volumetric_programs *progs,
                          const volumetric_quad *quad,
                          volumetric_target *t,
                          glid depth_tex);
void  volumetric_blur_cpu(const volumetric_blur *b,
                          float *img, float *scratch,
                          const float *depth, int w, int h);
#endif
