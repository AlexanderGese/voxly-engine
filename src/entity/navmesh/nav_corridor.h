#ifndef ENTITY_NAVMESH_NAV_CORRIDOR_H
#define ENTITY_NAVMESH_NAV_CORRIDOR_H

#include "nav_path.h"
#include "nav_grid.h"
#include "../../math/vec3.h"

// the per-tick follow layer that sits between a baked path and a mob's legs.
// a nav_path is a static list of waypoints; a corridor is the *moving* view of
// it: where the mover is along the path, which waypoint it's steering toward,
// and a steering vector to hand the locomotion code. it also does the small
// bookkeeping nav_path_next stubs out — advancing past a waypoint once you're
// close enough, and noticing when you've fallen off the path entirely.
//
// one of these per mob, cheap to keep. it borrows the path; it doesn't own it.

// how close (xz, blocks) the mover has to get to a waypoint to count as
// "reached" and advance. a hair under half a block so it triggers standing on
// the tile, not only dead-centre.
#define NAV_CORRIDOR_REACH   0.45f

// if the mover drifts further than this from the segment it's walking, we flag
// the corridor off-path so the ai can request a fresh search.
#define NAV_CORRIDOR_LEASH   2.5f

typedef struct {
    const nav_path *path;   // borrowed
    int   cursor;           // index of the waypoint we're heading toward
    int   off_path;         // 1 if the mover strayed past the leash
    float dist_left;        // straight-line blocks remaining, refreshed on tick
} nav_corridor;

// bind a corridor to a freshly found path. resets the cursor to the first
// real waypoint (index 1 if there's more than one, so we steer toward the next
// corner rather than the spot we're already standing on).
void nav_corridor_begin(nav_corridor *co, const nav_path *p);

// advance the cursor past any waypoints the mover has reached, recompute the
// remaining distance, and set the off-path flag. call once per ai tick before
// asking for a steer. returns 1 while there's still road ahead, 0 at the end.
int nav_corridor_update(nav_corridor *co, vec3 mover_pos);

// the world-space point the mover should steer toward right now (the current
// waypoint). returns mover_pos itself if the corridor is exhausted, so the
// caller can just stop.
vec3 nav_corridor_target(const nav_corridor *co, vec3 mover_pos);

// a normalised xz steering direction from mover toward the current target,
// flattened to the ground plane (mobs walk, they don't fly the path). zero
// vector when there's nothing to steer toward.
vec3 nav_corridor_steer(const nav_corridor *co, vec3 mover_pos);

// has the mover arrived at the goal (reached the last waypoint)?
int nav_corridor_done(const nav_corridor *co);

#endif
