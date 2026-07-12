#include "face_dir.h"

// normals indexed by face id. same convention as render/mesher.c.
static const int FACE_N[MB_NUM_FACES][3] = {
    { 1, 0, 0}, {-1, 0, 0},
    { 0, 1, 0}, { 0,-1, 0},
    { 0, 0, 1}, { 0, 0,-1},
};

// for each sweep axis: { pos_face, neg_face, u_axis, v_axis }.
// u/v are the two axes that arent the sweep axis. the ordering (u before v)
// is what fixes the texture orientation, so dont casually swap them — i did
// once and every top face came out mirrored.
static const int AXIS_TBL[MB_NUM_AXES][4] = {
    // sweep x -> faces +x/-x, plane is (z,y)
    { MB_FACE_PX, MB_FACE_NX, MB_AXIS_Z, MB_AXIS_Y },
    // sweep y -> faces +y/-y, plane is (x,z)
    { MB_FACE_PY, MB_FACE_NY, MB_AXIS_X, MB_AXIS_Z },
    // sweep z -> faces +z/-z, plane is (x,y)
    { MB_FACE_PZ, MB_FACE_NZ, MB_AXIS_X, MB_AXIS_Y },
};

void mb_face_normal(int face, int *nx, int *ny, int *nz) {
    *nx = FACE_N[face][0];
    *ny = FACE_N[face][1];
    *nz = FACE_N[face][2];
}

int mb_axis_pos_face(int axis) { return AXIS_TBL[axis][0]; }
int mb_axis_neg_face(int axis) { return AXIS_TBL[axis][1]; }

void mb_axis_plane(int axis, int *u_axis, int *v_axis) {
    *u_axis = AXIS_TBL[axis][2];
    *v_axis = AXIS_TBL[axis][3];
}

int mb_axis_dim(int axis) {
    switch (axis) {
    case MB_AXIS_X: return CHUNK_SIZE_X;
    case MB_AXIS_Y: return CHUNK_SIZE_Y;
    default:        return CHUNK_SIZE_Z;
    }
}

void mb_scatter(int axis, int slice, int u, int v, int *x, int *y, int *z) {
    int ua, va;
    mb_axis_plane(axis, &ua, &va);

    int coord[3];
    coord[axis] = slice;
    coord[ua]   = u;
    coord[va]   = v;

    *x = coord[MB_AXIS_X];
    *y = coord[MB_AXIS_Y];
    *z = coord[MB_AXIS_Z];
}
