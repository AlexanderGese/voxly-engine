#ifndef PLAYER_TOOLS_SPAWN_H
#define PLAYER_TOOLS_SPAWN_H
#include "tools_drop.h"
#include "../../math/vec3.h"
#include "../../math/rng.h"
#include "../../world/item.h"
// bridges a resolved drop_list into the engine's dropped-item world. given the
// block coords that just broke, this pops the right number of item entities at
// the block center with a little randomized scatter so they don't stack on one
// pixel. lives here (not in world/item) so the tools module owns the policy.
// spawn one drop_stack as N individual item entities. returns how many spawned
// (item_world has a hard cap, so it can be fewer than asked).
int  tools_spawn_stack(item_world *iw, vec3 block_center, drop_stack stack, rng *r);
// spawn an entire drop_list at a block. `bx,by,bz` are world block coords; the
// items appear centered in the block. returns total entities spawned.
int  tools_spawn_drops(item_world *iw, int bx, int by, int bz,
                       const drop_list *drops, rng *r);
vec3 tools_spawn_pop_vel(rng *r);
#endif
