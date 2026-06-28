#ifndef WORLD_STRUCTGEN_BOX_H
#define WORLD_STRUCTGEN_BOX_H

#include "structgen_types.h"

// integer-box helpers. footprint math, overlap rejection, and the little
// rotation/offset transform pieces use to place themselves relative to a hub.

structgen_box structgen_box_make(int x0, int y0, int z0, int x1, int y1, int z1);
// box rooted at (x,y,z) with sizes (w,h,d). exclusive max.
structgen_box structgen_box_at(int x, int y, int z, int w, int h, int d);

int structgen_box_width (const structgen_box *b);
int structgen_box_height(const structgen_box *b);
int structgen_box_depth (const structgen_box *b);
int structgen_box_volume(const structgen_box *b);

structgen_box structgen_box_translate(structgen_box b, int dx, int dy, int dz);
// grow/shrink on all sides by m (negative shrinks). xz and y separately.
structgen_box structgen_box_inset(structgen_box b, int mxz, int my);

int structgen_box_overlaps(const structgen_box *a, const structgen_box *b);
int structgen_box_contains_xz(const structgen_box *b, int x, int z);

// step one block out from a box center toward dir. used to find door cells.
void structgen_dir_step(structgen_dir d, int *dx, int *dz);
structgen_dir structgen_dir_rot(structgen_dir d, int quarters);   // +clockwise

#endif
