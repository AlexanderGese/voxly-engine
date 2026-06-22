#ifndef WORLD_OREGEN_DEBUG_H
#define WORLD_OREGEN_DEBUG_H

#include "oregen_types.h"
#include "oregen_buffer.h"

// stats + sanity checks for tuning the ore tables. none of this runs in the
// hot path; it exists so i can dump a histogram and eyeball whether diamond
// actually hugs the floor like the curve claims. wire it behind a debug key.

// per-y vertical histogram of emitted cells. bucket index is world y. fills
// out[0..max_y-1] with counts, returns the total counted (clamped to range).
int oregen_debug_height_hist(const oregen_buf *buf, int *out, int max_y);

// count cells per block id present in the buffer. out is indexed by block_id
// (caller sizes it BLOCK_COUNT). returns the number of distinct ids seen.
int oregen_debug_block_counts(const oregen_buf *buf, int *out, int out_len);

// crude density: fraction of cells that fall within [y_lo,y_hi]. handy to
// verify a band actually concentrates where it should.
float oregen_debug_band_fraction(const oregen_buf *buf, int y_lo, int y_hi);

// sanity pass over the seeded table: returns 0 if every ore has a sane band
// (y_min<=y_max, in [0,CHUNK_SIZE_Y]) and positive sizes, else a negative
// index-1 of the first offender. cheap startup assert fodder.
int oregen_debug_validate_table(void);

#endif
