#ifndef PLAYER_BUILDING_VALIDATE_H
#define PLAYER_BUILDING_VALIDATE_H

#include "building_types.h"
#include "../../math/vec3.h"
#include "../../math/aabb.h"
#include "../../world/world.h"

// placement rules. this is where "no you can't put that there" lives.
// returns one of the BPLACE_* codes. BPLACE_OK means go ahead.

// can `id` ever be placed by a player at all? rejects air, bedrock, fluids.
int building_is_placeable(block_id id);

// is the target cell free to receive a block? air and a few "replaceable"
// blocks (tall grass, flowers, snow layer) count as free.
int building_cell_replaceable(world *w, int x, int y, int z);

// does the cell sit inside the build column (0 <= y < CHUNK_SIZE_Y)?
int building_cell_in_bounds(int y);

// the collision aabb a block at (x,y,z) would occupy. plants/non-solid blocks
// return a zero box so they never block the player check.
aabb building_block_aabb(block_id id, int x, int y, int z);

// would placing `id` at the cell clip into the player's body box? `feet` is
// the player position (feet), matching player.pos in player.h.
int building_overlaps_player(block_id id, int x, int y, int z, vec3 feet);

// support check: plants need solid soil under them, ladders need a wall on
// the `face` they were placed against, etc. `face` is the clicked surface.
int building_has_support(world *w, block_id id, int x, int y, int z, int face);

// full gate. runs every rule in order and returns the first failure, or
// BPLACE_OK. `feet` is the player position for the overlap test.
int building_validate_place(world *w, block_id id, int x, int y, int z,
                            int face, vec3 feet);

#endif
