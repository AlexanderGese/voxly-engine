#ifndef RENDER_FXAA_SELFTEST_H
#define RENDER_FXAA_SELFTEST_H

// cpu-side sanity checks for the fxaa math. no gl context needed — they drive
// the C edge reference (fxaa_edge.c) over synthetic luma grids. run them at
// startup behind a debug flag or from a test harness. each returns 1 on pass,
// 0 on fail and logs the first failure.
//
// the edge-orientation test in particular earned its keep: i had the horz/vert
// gradient sums swapped and every vertical edge was being blurred sideways.

// a flat field triggers nothing.
int fxaa_selftest_flat_no_edge(void);

// a clean vertical step is detected and classified as a vertical edge.
int fxaa_selftest_vertical_edge(void);

// a clean horizontal step is detected and classified as horizontal.
int fxaa_selftest_horizontal_edge(void);

// the dark-region floor stops a tiny absolute contrast from triggering even
// when the relative contrast is huge.
int fxaa_selftest_dark_floor(void);

// params derive: reciprocal threshold and subpix quad come out right, and
// sanitize clamps an out-of-range quality index.
int fxaa_selftest_params_derive(void);

// quality reach is monotonic non-decreasing across the presets.
int fxaa_selftest_quality_reach(void);

// the edge search returns a bounded offset in [-0.5, 0.5] on a real edge.
int fxaa_selftest_search_bounded(void);

// settings: off preset disables, and classify round-trips a known preset.
int fxaa_selftest_settings_roundtrip(void);

// history: running average matches a brute-force mean and stays in [0,1].
int fxaa_selftest_history_avg(void);

// runs everything. returns the count of failures (0 == all good).
int fxaa_selftest_run_all(void);

#endif
