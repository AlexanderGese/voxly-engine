#ifndef RENDER_TONEMAP_SELFTEST_H
#define RENDER_TONEMAP_SELFTEST_H

// cpu-side sanity checks for the tonemap math. no gl involved, so this runs
// fine in a headless build / ci. each returns 1 on pass, 0 on fail and logs
// the reason.

int tonemap_selftest_curves_monotonic(void);
int tonemap_selftest_curve_bounds(void);
int tonemap_selftest_exposure_adapt(void);
int tonemap_selftest_grade_identity(void);
int tonemap_selftest_lut_identity(void);
int tonemap_selftest_lut_blend(void);

// run them all. returns the number of failures (0 == all good).
int tonemap_selftest_run_all(void);

#endif
