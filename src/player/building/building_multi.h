#ifndef PLAYER_BUILDING_MULTI_H
#define PLAYER_BUILDING_MULTI_H
#include "building_types.h"
#include "building_history.h"
#include "../../math/vec3.h"
#include "../../world/world.h"
// bulk edits: drag a line, fill a box. the single-block place/break in
// building_place.c is the unit op; this stacks it. every cell still goes
// through building_validate_place so you can't fill yourself into a wall or
// across bedrock. each cell is recorded as its own history edit so undo peels
// them back one at a time (annoying for a 200-block fill, fine for now).
#define BUILDING_MULTI_MAX 4096   // hard cap so a fat box can't OOM us
// a planned multi-edit: the list of cells we intend to touch, resolved but not
// yet applied. lets the renderer ghost the whole shape before commit.
typedef struct {
    int   cells[BUILDING_MULTI_MAX][3];
    int   count;
    block_id id;       // block to place (BLOCK_AIR for an erase fill)
} building_plan;
// --- line (the click-drag-to-place stroke) ---------------------------------
// plan a straight voxel line from cell a to cell b (inclusive), 3d DDA so
// diagonals look right. dedups consecutive cells. returns cell count.
int building_plan_line(building_plan *plan, block_id id,
                       int ax, int ay, int az, int bx, int by, int bz);
// --- box (two corners) ------------------------------------------------------
// plan a solid axis-aligned box spanning the two corner cells inclusive.
// if `hollow` is set, only the shell (faces) is included. returns cell count,
// or -1 if the box would exceed BUILDING_MULTI_MAX.
int building_plan_box(building_plan *plan, block_id id,
                      int ax, int ay, int az, int bx, int by, int bz,
                      int hollow);
// --- commit -----------------------------------------------------------------
// apply a place-plan: for each cell, run the place gate and write if it passes.
// skips cells that fail validation rather than aborting the whole stroke.
int building_plan_commit(world *w, building_history *hist,
                         const building_plan *plan, vec3 feet);
int building_plan_erase(world *w, building_history *hist,
                        const building_plan *plan);
void building_plan_clear(building_plan *plan);
#endif
