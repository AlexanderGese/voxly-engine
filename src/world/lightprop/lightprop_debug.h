#ifndef WORLD_LIGHTPROP_DEBUG_H
#define WORLD_LIGHTPROP_DEBUG_H

#include "lightprop_types.h"

// optional verification + stats. compiled in always but only ever called from
// debug builds / the F3-style overlay. handy when a torch fix "works on my
// machine" but someone reports a dark seam at a chunk border.

typedef struct {
    int lit_block;     // cells with block light > 0
    int lit_sky;       // cells with sky light > 0
    int max_block;     // brightest block-lit cell found
    int max_sky;
    int suspect;       // cells violating the falloff invariant (see below)
} lp_stats;

// walk a chunk and tally light. cheap, O(volume).
void lp_collect_stats(world *w, chunk *c, lp_stats *out);

// invariant check: every transparent cell's light must be >= (best neighbour
// light - step cost). a cell brighter than that with no local source means a
// removal left a ghost; a cell darker means a flood didn't reach it. returns the
// count of violations for `ch` and logs the first few.
int lp_verify_chunk(world *w, chunk *c, lp_channel ch);

// pretty-print a vertical slice (one z row) to the log for eyeballing.
void lp_dump_slice(world *w, chunk *c, int lz, lp_channel ch);

#endif
