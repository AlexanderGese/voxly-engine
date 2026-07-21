#ifndef RENDER_TONEMAP_DEBUG_H
#define RENDER_TONEMAP_DEBUG_H
#include <stddef.h>
#include "tonemap.h"
// debug glue for the tonemap stage: a one-liner summary for the F3-style
// overlay, the split-view shader runner, and a couple of formatting helpers.
// nothing here is load-bearing; it just reads the live state.
// write a human readable summary (curve, exposure, grade, lut) into buf,
// truncated to cap. returns snprintf-style would-have-written length.
int  tonemap_debug_summary(const tonemap *tm, char *buf, size_t cap);
// log the summary at info level. handy after toggling a look.
void tonemap_debug_log(const tonemap *tm);
void tonemap_debug_run_split(const tonemap *tm, glid scene_tex,
                             glid dst, int dst_w, int dst_h, float seam);
int  tonemap_debug_cycle_curve(tonemap *tm);
#endif
