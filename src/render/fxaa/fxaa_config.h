#ifndef RENDER_FXAA_CONFIG_H
#define RENDER_FXAA_CONFIG_H

// compile-time tunables for the fxaa post pass. this is the cheap edge
// antialiasing we run after tonemapping, before the ui goes on top. it's a
// single fullscreen pass: detect luma edges, walk along them, blend. no msaa
// samples, no taa history. lo-fi but it does the job and costs ~nothing.

// fxaa works on luma, not color. the classic implementation reads luma from
// the alpha channel of the source texture (filled by a tiny prepass) so the
// main pass only needs one fetch per neighbour. we do the same.
#define FXAA_LUMA_IN_ALPHA      1

// edge detect threshold. the local contrast (max luma - min luma in the
// 3x3 plus-neighbourhood) has to clear this fraction of the max luma before
// we bother antialiasing the texel. lower = more edges caught = softer image.
// 0.333 -> very aggressive (lots of blur)
// 0.250 -> default, "high quality"
// 0.125 -> only the harshest edges
#define FXAA_DEFAULT_EDGE_THRESHOLD     0.166f

// absolute floor on contrast. in near-black regions the relative threshold
// goes nuts (tiny numbers), so we also require this much absolute luma delta.
// keeps fxaa from chewing on sensor-noise-grade gradients in the dark.
#define FXAA_DEFAULT_EDGE_THRESHOLD_MIN 0.0833f

// subpixel aliasing removal amount. fxaa estimates how much a texel is a
// thin sub-pixel feature (single-pixel lines, dots) and blurs those toward
// the local average. 1.0 = full blur, 0.0 = off. too high looks smeared.
#define FXAA_DEFAULT_SUBPIX             0.75f

// the edge search walks up to this many texels in each direction looking for
// the end of the edge span. more steps = better long shallow edges but more
// texture fetches. the quality table (fxaa_quality.c) controls the step sizes.
#define FXAA_MAX_SEARCH_STEPS   12

// quality presets. these pick how many search steps run and how far each one
// jumps. higher preset = more steps + finer near, coarser far.
#define FXAA_QUALITY_LOW        0
#define FXAA_QUALITY_MEDIUM     1
#define FXAA_QUALITY_HIGH       2
#define FXAA_QUALITY_EXTREME    3
#define FXAA_QUALITY_COUNT      4
#define FXAA_DEFAULT_QUALITY    FXAA_QUALITY_HIGH

// rgb -> luma weights. perceptual (rec.601-ish). fxaa traditionally uses just
// green as a luma approximation for speed, but we compute a weighted luma in
// the prepass once so we can afford the better version.
#define FXAA_LUMA_R     0.299f
#define FXAA_LUMA_G     0.587f
#define FXAA_LUMA_B     0.114f

// the green-only fast luma the reference shader uses inside the main loop.
// kept as a separate knob because matching the prepass weighting to the
// loop weighting matters for edge symmetry.
#define FXAA_FAST_LUMA_USES_GREEN  1

#endif
