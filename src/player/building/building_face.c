#include "building_face.h"

#include <math.h>

// normals indexed by BFACE_*. keep in lockstep with the enum.
static const int FACE_N[BFACE_COUNT][3] = {
    {  1,  0,  0 }, // +x
    { -1,  0,  0 }, // -x
    {  0,  1,  0 }, // +y
    {  0, -1,  0 }, // -y
    {  0,  0,  1 }, // +z
    {  0,  0, -1 }, // -z
};

void building_face_normal(int face, int *dx, int *dy, int *dz) {
    if (face < 0 || face >= BFACE_COUNT) { *dx = *dy = *dz = 0; return; }
    *dx = FACE_N[face][0];
    *dy = FACE_N[face][1];
    *dz = FACE_N[face][2];
}

vec3 building_face_normal_v(int face) {
    int dx, dy, dz;
    building_face_normal(face, &dx, &dy, &dz);
    return vec3_new((float)dx, (float)dy, (float)dz);
}

int building_face_opposite(int face) {
    // flip the low bit. enum is laid out as pairs (px,nx),(py,ny),(pz,nz).
    if (face < 0 || face >= BFACE_COUNT) return -1;
    return face ^ 1;
}

void building_face_ring(int face, int out[4]) {
    // the four faces perpendicular to this one. just enumerate everything
    // that isn't `face` or its opposite. order is stable but arbitrary.
    int opp = building_face_opposite(face);
    int n = 0;
    for (int f = 0; f < BFACE_COUNT && n < 4; f++) {
        if (f == face || f == opp) continue;
        out[n++] = f;
    }
    while (n < 4) out[n++] = -1; // shouldn't happen but be safe
}

int building_face_is_vertical(int face) {
    return face == BFACE_PY || face == BFACE_NY;
}

void building_face_adjacent(int hx, int hy, int hz, int face,
                            int *ax, int *ay, int *az) {
    int dx, dy, dz;
    building_face_normal(face, &dx, &dy, &dz);
    *ax = hx + dx;
    *ay = hy + dy;
    *az = hz + dz;
}

int building_face_from_dir(vec3 dir) {
    // we want the face whose outward normal most opposes the look dir, i.e.
    // the face you'd be staring at. so minimise dot(normal, dir).
    int best = BFACE_PY;
    float best_dot = 1e30f;
    for (int f = 0; f < BFACE_COUNT; f++) {
        vec3 n = building_face_normal_v(f);
        float d = vec3_dot(n, dir);
        if (d < best_dot) {
            best_dot = d;
            best = f;
        }
    }
    return best;
}

vec3 building_face_center(int x, int y, int z, int face) {
    // block center is (x+.5, y+.5, z+.5); push half a unit along the normal.
    vec3 c = vec3_new(x + 0.5f, y + 0.5f, z + 0.5f);
    vec3 n = building_face_normal_v(face);
    return vec3_add(c, vec3_scale(n, 0.5f));
}
