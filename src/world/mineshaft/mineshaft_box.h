#ifndef WORLD_MINESHAFT_BOX_H
#define WORLD_MINESHAFT_BOX_H

#include "mineshaft_types.h"

// integer-box helpers. footprint math, the cell->world transform, and the
// direction stepping the maze uses to find neighbours. mirrors structgen_box
// closely; kept separate so the two subsystems don't have to link each other.

mineshaft_box mineshaft_box_make(int x0, int y0, int z0, int x1, int y1, int z1);
// box rooted at (x,y,z) with sizes (w,h,d). exclusive max.
mineshaft_box mineshaft_box_at(int x, int y, int z, int w, int h, int d);

int mineshaft_box_width (const mineshaft_box *b);
int mineshaft_box_height(const mineshaft_box *b);
int mineshaft_box_depth (const mineshaft_box *b);
int mineshaft_box_volume(const mineshaft_box *b);

mineshaft_box mineshaft_box_translate(mineshaft_box b, int dx, int dy, int dz);
mineshaft_box mineshaft_box_inset(mineshaft_box b, int mxz, int my);
mineshaft_box mineshaft_box_union(mineshaft_box a, mineshaft_box b);

int mineshaft_box_overlaps(const mineshaft_box *a, const mineshaft_box *b);
int mineshaft_box_contains(const mineshaft_box *b, int x, int y, int z);

// unit step for a cardinal direction, in grid or world units.
void mineshaft_dir_step(mineshaft_dir d, int *dx, int *dz);
mineshaft_dir mineshaft_dir_rot(mineshaft_dir d, int quarters);   // +clockwise
mineshaft_dir mineshaft_dir_opposite(mineshaft_dir d);
int mineshaft_dir_link_bit(mineshaft_dir d);

#endif
