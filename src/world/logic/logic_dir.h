#ifndef WORLD_LOGIC_DIR_H
#define WORLD_LOGIC_DIR_H

#include "logic_types.h"

// direction helpers. the wire layer is full of "step to the neighbour in dir d"
// so this is its own tiny file to keep the arithmetic in one place.

// unit offset for each logic_dir. index with the enum directly.
extern const int LOGIC_DX[LOGIC_DIR_COUNT];
extern const int LOGIC_DY[LOGIC_DIR_COUNT];
extern const int LOGIC_DZ[LOGIC_DIR_COUNT];

// step (x,y,z) one block along d. writes through the out pointers.
void logic_dir_step(logic_dir d, int x, int y, int z, int *ox, int *oy, int *oz);

// the opposite direction. PX<->NX etc. (xor the low bit, basically)
logic_dir logic_dir_opposite(logic_dir d);

// is this a horizontal direction? gates/repeaters only face horizontally.
int logic_dir_is_horizontal(logic_dir d);

// rotate a horizontal direction 90 degrees clockwise (viewed from +y).
// passing a vertical dir returns it unchanged - nothing sensible to do.
logic_dir logic_dir_rotate_cw(logic_dir d);

// the two horizontal directions perpendicular to d. used to find a gate's
// side inputs. writes LOGIC_DIR_COUNT into a slot if d is vertical.
void logic_dir_perp(logic_dir d, logic_dir *left, logic_dir *right);

// pick the dir that points from (ax,ay,az) toward an adjacent (bx,by,bz).
// returns LOGIC_DIR_COUNT if the cells are not axis-adjacent.
logic_dir logic_dir_between(int ax, int ay, int az, int bx, int by, int bz);

// short name for the debug dump, e.g. "+x". returns "?" for LOGIC_DIR_COUNT.
const char *logic_dir_name(logic_dir d);

#endif
