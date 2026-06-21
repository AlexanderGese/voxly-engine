#ifndef WORLD_LOGIC_TRACE_H
#define WORLD_LOGIC_TRACE_H

#include "logic_types.h"
#include "logic_grid.h"

// net tracing. given a starting wire cell, walk the connected wire blob it
// belongs to and report it: the cells in it, where the power enters, and how
// far the strongest source reaches. used by the debug overlay (highlight the
// net under the crosshair) and to sanity-check that a line actually conducts
// end to end.
//
// "connected" here means wire-to-wire adjacency on the six axes. emitters that
// touch the blob are recorded as feed points but are not part of the blob.

#define LOGIC_TRACE_MAX 1024   // hard cap on cells visited per trace

typedef struct {
    int      cell_count;        // wire cells in the connected blob
    int      feed_count;        // distinct emitters feeding it
    uint8_t  strongest_feed;    // highest emitter output touching the blob
    uint8_t  max_reach;         // strongest power level present on any wire
    int      lit;               // 1 if any wire in the blob carries power
    int      truncated;         // 1 if we hit LOGIC_TRACE_MAX and stopped

    // the cells we walked, in visit order. coords only; caller can re-look-up.
    int      coords[LOGIC_TRACE_MAX][3];
} logic_trace;

// trace the wire blob containing (x,y,z). if that coord isn't a wire the trace
// comes back empty (cell_count 0). returns the number of wire cells visited.
int logic_trace_net(logic_grid *g, int x, int y, int z, logic_trace *out);

// shortest wire-distance (in steps) from a coord back to the nearest emitter,
// or -1 if the blob has no feed at all. a quick "is this powered and from
// where" probe without filling a whole logic_trace.
int logic_trace_distance_to_source(logic_grid *g, int x, int y, int z);

#endif
