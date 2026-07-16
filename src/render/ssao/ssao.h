#ifndef RENDER_SSAO_H
#define RENDER_SSAO_H
// screen-space ambient occlusion. umbrella header for the ssao subsystem.
// include this and you get the whole thing.
//
// this is the "ssaox_" module under render/ssao/. there's an older stub at
// render/post/ssao.c using the bare "ssao_" prefix — that one predates this
// and is on the chopping block once renderer.c switches over. dont mix them.
//
// the pieces:
// ssao_config   tunables + texture unit assignments
// ssao_kernel   hemisphere sample set (cpu, packed for the shader)
// ssao_noise    rotation noise tile (cpu copy + gl texture)
// ssao_sample   the core projection + range-check math (cpu reference)
// ssao_gbuffer  borrowed geometry-pass inputs + view-pos reconstruction
// ssao_fsquad   fullscreen triangle
// ssao_blur     box blur to kill the noise grain (gpu + cpu)
// ssao_pass     the gpu pass orchestrator (kernel+noise+gbuffer -> occl)
// ssao_compute  the full cpu reference compute, for tests/headless
// ssao_settings user-facing quality presets + clamping
// ssao_debug    apply settings to a pass + readback stats
//
// typical wiring in the renderer:
// ssaox_pass_init(&p, w, h, 16);
// ... per frame, after the geometry pass ...
// ssaox_gbuffer g; ssaox_gbuffer_clear(&g); fill g.tex_*, g.proj, ...
// ssaox_pass_run(&p, &g);
#include "ssao_config.h"
#include "ssao_kernel.h"
#include "ssao_kernel_stats.h"
#include "ssao_noise.h"
#include "ssao_sample.h"
#include "ssao_gbuffer.h"
#include "ssao_fsquad.h"
#include "ssao_blur.h"
#include "ssao_pass.h"
#include "ssao_compute.h"
#include "ssao_settings.h"
#include "ssao_debug.h"
#endif
