#ifndef RENDER_MESHBUILD_FACE_DIR_H
#define RENDER_MESHBUILD_FACE_DIR_H
// face direction bookkeeping. greedy meshing sweeps along an axis and on each
// slice it considers two opposing faces (e.g. +y above the cell and -y below).
// this little module keeps all the "which axis maps to which face / which two
#include "mb_config.h"
enum {
    MB_FACE_PX = 0,
    MB_FACE_NX,
    MB_FACE_PY,
    MB_FACE_NY,
    MB_FACE_PZ,
    MB_FACE_NZ,
}
;
void mb_face_normal(int face, int *nx, int *ny, int *nz);
int  mb_axis_pos_face(int axis);
int  mb_axis_neg_face(int axis);
void mb_axis_plane(int axis, int *u_axis, int *v_axis);
void mb_scatter(int axis, int slice, int u, int v, int *x, int *y, int *z);
int  mb_axis_dim(int axis);
#endif
