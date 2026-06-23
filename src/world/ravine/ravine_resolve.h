#ifndef WORLD_RAVINE_RESOLVE_H
#define WORLD_RAVINE_RESOLVE_H

#include "ravine_field.h"
#include "ravine_path.h"
#include "ravine_types.h"

// the mask pass: turns a spline path into a filled field. for every cell in the
// padded region it finds the nearest point on the centreline, runs the
// cross-section profile against that distance, and writes cut depth, floor
// height and kind. a face-ripple noise (wall_jitter) nudges the effective
// distance so the canyon edge isnt a clean mathematical offset of the spline.
//
// the floor of a channel cell is the lower of the profile's implied floor
// (surface - cut) and the spline's own descending floor, clamped above
// min_floor_y so we never bite into the bedrock guard. taking the lower of the
// two is what lets the channel trench a real depth while still following the
// downhill roll the spline laid out.

typedef struct {
    int cells_cut;     // cells with cut > 0 (centre footprint only)
    int wall_cells;
    int floor_cells;
    int rim_cells;
    int deepest;       // max cut depth seen, blocks
} ravine_mask_stats;

// resolve one path into the field. the field must already have its surface
// filled. accumulates onto whatever the field holds, so overlapping ravines
// take the deeper cut. returns per-region stats.
ravine_mask_stats ravine_resolve(ravine_field *f, const ravine_path *path,
                                 const ravine_params *p);

#endif
