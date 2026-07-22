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
int volumetric_selftest_transmittance(void);
int volumetric_selftest_medium_clamp(void);
int volumetric_selftest_march_energy(void);
int volumetric_selftest_march_shadowed(void);
int volumetric_selftest_frustum_clip(void);
int volumetric_selftest_dither_range(void);
int volumetric_selftest_run_all(void);
#endif
