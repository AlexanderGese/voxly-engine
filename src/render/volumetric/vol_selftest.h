#ifndef RENDER_VOLUMETRIC_SELFTEST_H
#define RENDER_VOLUMETRIC_SELFTEST_H

// cpu-side sanity checks for the volumetric math. no gl context needed, so the
// engine can run these at startup behind a debug flag or from a test harness.
// each returns 1 on pass, 0 on fail and logs the first thing it didn't like.
//
// the energy-conservation check actually earned its keep: an early version of
// the marcher advanced transmittance *before* accumulating, double-dimming the
// first slab. these caught it.

// hg phase integrates (numerically, over the sphere) to ~1 for a few g values.
int volumetric_selftest_phase_normalized(void);

// transmittance is monotonic decreasing and exp(-e*0) == 1.
int volumetric_selftest_transmittance(void);

// medium_init forces extinct >= scatter and clamps g away from +-1.
int volumetric_selftest_medium_clamp(void);

// a fully-lit march never produces more light than the single-scatter ceiling,
// i.e. it doesn't manufacture energy.
int volumetric_selftest_march_energy(void);

// a fully-shadowed march produces zero light but still attenuates the ray.
int volumetric_selftest_march_shadowed(void);

// ray/box clip: a ray through the box centre enters and exits, a ray that
// misses reports no hit, and the surface distance caps t_far.
int volumetric_selftest_frustum_clip(void);

// dither matrix values stay in [0,1) and the bayer tile has no duplicates.
int volumetric_selftest_dither_range(void);

// runs everything. returns the count of failures (0 == all good).
int volumetric_selftest_run_all(void);

#endif
