#ifndef RENDER_DECALS_DEBUG_H
#define RENDER_DECALS_DEBUG_H

#include "decals.h"

// debug helpers for the decal subsystem. stats text + projector box overlay.
// kept out of the hot path; only the debug overlay calls in here.

typedef struct {
    int   live;              // currently-alive decals
    int   visible;           // survived last cull
    int   drawn;             // instances actually stamped last frame
    int   culled_frustum;
    int   culled_dist;
    int   culled_alpha;
    float fill_ratio;        // live / DECALS_MAX
    int   phase_count[4];    // by decals_phase
} decals_debug_stats;

// snapshot the current system stats. cheap; walks the pool once.
void decals_debug_collect(const decals_system *s, decals_debug_stats *out);

// format the stats into a one-liner for the hud. writes at most `cap` bytes,
// always nul-terminates. returns the length written.
int  decals_debug_format(const decals_debug_stats *st, char *buf, int cap);

// emit the 12 edges of every visible projector box into a line buffer the
// debugdraw subsystem can consume. writes pairs of vec3 endpoints, up to
// `max_lines` lines; returns the number of lines written.
int  decals_debug_box_lines(const decals_system *s, vec3 *out, int max_lines);

#endif
