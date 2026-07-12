#ifndef RENDER_MESHBUILD_FACE_DIR_H
#define RENDER_MESHBUILD_FACE_DIR_H

// face direction bookkeeping. greedy meshing sweeps along an axis and on each
// slice it considers two opposing faces (e.g. +y above the cell and -y below).
// this little module keeps all the "which axis maps to which face / which two
// in-plane axes" tables in one place so the rest of the code reads cleanly.

#include "mb_config.h"

// the 6 face directions, matching the order render/mesher.c already uses.
// keep this enum and the d[] table in mesher.c in sync if you ever touch it.
enum {
    MB_FACE_PX = 0,
    MB_FACE_NX,
    MB_FACE_PY,
    MB_FACE_NY,
    MB_FACE_PZ,
    MB_FACE_NZ,
};

// unit normal for a face.
void mb_face_normal(int face, int *nx, int *ny, int *nz);

// the positive-direction face for a sweep axis (0=x,1=y,2=z).
int  mb_axis_pos_face(int axis);
int  mb_axis_neg_face(int axis);

// given the sweep axis, return the two in-plane axes (u then v). these are the
// axes the greedy rectangle grows along. e.g. sweeping y gives u=x, v=z.
void mb_axis_plane(int axis, int *u_axis, int *v_axis);

// scatter a (slice, u, v) coordinate triple back into x/y/z given the sweep
// axis. `slice` is the coordinate along the sweep axis.
void mb_scatter(int axis, int slice, int u, int v, int *x, int *y, int *z);

// dimension of the chunk along a given axis (0=x,1=y,2=z).
int  mb_axis_dim(int axis);

#endif
