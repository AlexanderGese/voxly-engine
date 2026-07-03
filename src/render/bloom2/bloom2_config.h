#ifndef RENDER_BLOOM2_CONFIG_H
#define RENDER_BLOOM2_CONFIG_H

// tunables for the bloom2 post chain. the old post/bloom.c was a two-pass
// toy; this one does a proper downsample/upsample mip chain like the
// "next generation post processing in cod:aw" talk everyone copies.

// how many mip levels to build for the blur pyramid. each level is half
// the resolution of the one above. 6 is plenty for 1080p, beyond that the
// smallest mips are like 4px wide and contribute basically nothing.
#define BLOOM2_MAX_MIPS        7

// dont build mips smaller than this on either axis. tiny mips cause
// flickery fireflies and arent worth the bandwidth.
#define BLOOM2_MIN_MIP_DIM     4

// default bright-pass threshold (luma). pixels dimmer than this contribute
// nothing. soft knee softens the edge so it doesnt pop.
#define BLOOM2_DEFAULT_THRESHOLD   1.0f
#define BLOOM2_DEFAULT_KNEE        0.5f

// how strongly the bloom is added back over the scene at composite.
#define BLOOM2_DEFAULT_INTENSITY   0.08f

// radius scale for the upsample tent filter, in mip texels. bigger = softer
// and wider glow. 1.0 is the "textbook" value.
#define BLOOM2_DEFAULT_RADIUS      1.0f

// clamp very bright input so a single nan/inf texel cant blow out the whole
// frame. this is a luma clamp applied in the bright pass.
#define BLOOM2_FIREFLY_CLAMP       16.0f

// the cpu-side gaussian we precompute for the separable fallback path (the
// gpu mip chain is the main path, but we keep a separable blur around for
// when the driver hates our linear-sampling tricks). this is the half-width.
#define BLOOM2_GAUSS_RADIUS        5
#define BLOOM2_GAUSS_TAPS          (BLOOM2_GAUSS_RADIUS * 2 + 1)

#endif
