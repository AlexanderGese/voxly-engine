#ifndef WORLD_LOGIC_CLOCK_H
#define WORLD_LOGIC_CLOCK_H
#include "logic_types.h"
// clock + edge detection. redstone loves to oscillate - a torch feeding itself
// through an odd number of inverters is a clock, and a tight loop of them is a
// frame-killer. this module watches individual cells for how often they flip
// and lets the net throttle anything that toggles too fast, while still letting
// a deliberate, slow clock run.
//
// it's a small ring of recent flip ticks per watched cell. if a cell flips more
// than LOGIC_CLOCK_BURST times within LOGIC_CLOCK_WINDOW ticks we declare it a
// runaway and ask the net to back off (longer reschedule delay).
#define LOGIC_CLOCK_WINDOW 8   // ticks we look back over
#define LOGIC_CLOCK_BURST  6   // flips in the window that count as "too fast"
#define LOGIC_CLOCK_RING   8   // remembered flip timestamps per watcher
// tracks the recent flip history of one cell, keyed by coord.
typedef struct {
    uint64_t key;
    uint32_t flips[LOGIC_CLOCK_RING];
    uint8_t  head;     // next write slot
    uint8_t  count;    // valid entries (<= LOGIC_CLOCK_RING)
    uint8_t  last_on;  // previous boolean state, for edge detection
    uint8_t  throttle; // current backoff delay in ticks (1 = none)
} logic_clock_watch;
// a fixed pool of watchers. small on purpose - only a handful of cells in a
// world are ever fast enough to need watching.
#define LOGIC_CLOCK_MAX 64
typedef struct {
    logic_clock_watch watch[LOGIC_CLOCK_MAX];
    int count;
} logic_clock;
void logic_clock_init(logic_clock *ck);
// note that a cell is at state `on` at tick `now`. if this is an edge it gets
// recorded. returns the reschedule delay the net should use for this cell: 1
// normally, larger if the cell is being throttled as a runaway.
uint32_t logic_clock_observe(logic_clock *ck, uint64_t key, int on, uint32_t now);
// is the cell at `key` currently flagged as a runaway clock?
int logic_clock_is_runaway(const logic_clock *ck, uint64_t key);
// how many cells are being actively throttled right now.
int logic_clock_throttled_count(const logic_clock *ck);
#endif
