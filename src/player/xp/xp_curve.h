#ifndef PLAYER_XP_XP_CURVE_H
#define PLAYER_XP_XP_CURVE_H

// level <-> experience conversions.
//
// "xp_to_next" is how much you need to clear the *current* level.
// "xp_total_for" is the cumulative xp at the start of a given level.
// both are integer; xp is never fractional in this engine.

#include <stdint.h>

// xp required to advance from `level` to `level+1`.
int       xp_curve_to_next(int level);

// cumulative xp at the boundary where you first reach `level`.
// xp_total_for(0) == 0 by definition.
int64_t   xp_curve_total_for(int level);

// inverse: given a cumulative total, what level does it land on, and how
// much progress (0..1) into the next level. either out-pointer may be null.
int       xp_curve_level_at(int64_t total, float *progress_out);

// raw progress within the current level, in xp units (not normalized).
int       xp_curve_progress_in(int64_t total, int level);

#endif
