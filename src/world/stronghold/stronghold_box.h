#ifndef WORLD_STRONGHOLD_BOX_H
#define WORLD_STRONGHOLD_BOX_H

#include "stronghold_types.h"

// integer-box helpers. footprint math, overlap rejection, and the little
// rotation/offset transforms rooms use to place themselves relative to a hub.
// deliberately a near-clone of structgen_box: the maze does the same kind of
// jigsaw bookkeeping, no point inventing a second vocabulary.

stronghold_box stronghold_box_make(int x0, int y0, int z0, int x1, int y1, int z1);
// box rooted at (x,y,z) with sizes (w,h,d). exclusive max.
stronghold_box stronghold_box_at(int x, int y, int z, int w, int h, int d);

int stronghold_box_width (const stronghold_box *b);
int stronghold_box_height(const stronghold_box *b);
int stronghold_box_depth (const stronghold_box *b);
int stronghold_box_volume(const stronghold_box *b);

// center, floored. handy for door projection and portal placement.
void stronghold_box_center(const stronghold_box *b, int *cx, int *cy, int *cz);

stronghold_box stronghold_box_translate(stronghold_box b, int dx, int dy, int dz);
// grow/shrink on all sides by m (negative shrinks). xz and y separately.
stronghold_box stronghold_box_inset(stronghold_box b, int mxz, int my);
// smallest box containing both. running-union for plan bounds.
stronghold_box stronghold_box_union(stronghold_box a, stronghold_box b);

int stronghold_box_overlaps(const stronghold_box *a, const stronghold_box *b);
// overlaps but with a margin of `pad` blocks; used to keep rooms from kissing.
int stronghold_box_overlaps_pad(const stronghold_box *a, const stronghold_box *b, int pad);
int stronghold_box_contains(const stronghold_box *b, int x, int y, int z);
int stronghold_box_contains_xz(const stronghold_box *b, int x, int z);

// unit step in a direction. -z/+x/+z/-x.
void stronghold_dir_step(stronghold_dir d, int *dx, int *dz);
stronghold_dir stronghold_dir_rot(stronghold_dir d, int quarters);   // +clockwise
stronghold_dir stronghold_dir_opposite(stronghold_dir d);

#endif
