#ifndef WORLD_LIGHTPROP_BORDER_H
#define WORLD_LIGHTPROP_BORDER_H
#include "lightprop_types.h"
#include "lightprop_queue.h"
// cross-chunk seam stitching. chunks get lit one at a time, often before their
// neighbours exist. when a neighbour finally loads, the seam between them is
// wrong: light that should have spilled across stops dead at the old border, and
// you get a dark vertical stripe right at x=0 / z=0 of the new chunk. classic
// voxel-engine bug, eternally rediscovered.
//
// the fix is cheap: when two chunks become neighbours, walk their shared face and
// re-seed the bfs from whichever side is brighter, in both channels. flood()
// only raises, so re-running it across an already-lit region is safe and idem-
// potent -- it just patches the seam and stops.
// stitch the seam between chunk `c` and its neighbour in direction `face`
// (0=+x 1=-x 4=+z 5=-z; vertical faces don't exist between chunks). no-ops if
// that neighbour isn't loaded.
void lp_border_stitch(world *w, chunk *c, int face);
// stitch all four horizontal seams of `c`. call this right after a chunk's own
// lighting is computed and its neighbour pointers are set.
void lp_border_stitch_all(world *w, chunk *c);
// after a chunk is unloaded/reloaded, its border light may be stale relative to
// survivors. re-pull light INTO `c` from every loaded neighbour. returns the
// number of cells it actually brightened (0 means the seam was already fine).
int  lp_border_pull(world *w, chunk *c);
#endif
