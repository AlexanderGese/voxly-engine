#ifndef RENDER_VOLUMETRIC_H
#define RENDER_VOLUMETRIC_H
// volumetric light / god rays. umbrella header for the subsystem — include
// this and you get the whole pipeline.
//
// we raymarch the camera frustum against the sun's shadow map, accumulating
// in-scattered light with a henyey-greenstein phase, dither the per-pixel ray
// start to trade banding for noise, then bilateral-blur and composite the
// shafts additively over the lit scene. everything runs at reduced resolution
// because the result is low frequency.
//
// the pieces:
// vol_config    tunables + texture unit assignments
// vol_phase     hg phase + transmittance math (cpu reference)
// vol_medium    time-of-day -> validated medium + tint profile
// vol_dither    bayer start-offset jitter (cpu copy + gl texture)
// vol_frustum   ray/box clipping so we don't march empty space
// vol_raymarch  the cpu reference marcher, for tests/headless + fallback
// vol_quad      fullscreen triangle
// vol_target    reduced-res ping-pong scatter buffers
// vol_programs  the three gl programs (march / blur / composite)
// vol_blur      separable bilateral blur (gpu + cpu)
// vol_params    runtime-mutable, clamped tunables
// vol_pass      the gpu pass orchestrator
// vol_debug     readback stats for the hud
#include "vol_config.h"
#include "vol_phase.h"
#include "vol_medium.h"
#include "vol_dither.h"
#include "vol_frustum.h"
#include "vol_raymarch.h"
#include "vol_quad.h"
#include "vol_target.h"
#include "vol_programs.h"
#include "vol_blur.h"
#include "vol_params.h"
#include "vol_pass.h"
#include "vol_debug.h"
#include "vol_selftest.h"
#endif
