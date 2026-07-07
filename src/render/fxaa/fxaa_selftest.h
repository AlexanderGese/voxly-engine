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
int fxaa_selftest_dark_floor(void);
int fxaa_selftest_params_derive(void);
int fxaa_selftest_quality_reach(void);
int fxaa_selftest_search_bounded(void);
int fxaa_selftest_settings_roundtrip(void);
int fxaa_selftest_history_avg(void);
int fxaa_selftest_run_all(void);
#endif
