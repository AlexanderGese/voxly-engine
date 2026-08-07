#ifndef PLAYER_BUILDING_FACE_H
#define PLAYER_BUILDING_FACE_H

#include "building_types.h"
#include "../../math/vec3.h"

// face geometry helpers. all the "which way is this face pointing" math
// lives here so the rest of the module never hardcodes a normal again.

// integer normal of a face, e.g. BFACE_PY -> (0,1,0).
void building_face_normal(int face, int *dx, int *dy, int *dz);

// float normal, handy for dotting against the camera dir.
vec3 building_face_normal_v(int face);

// opposite face. BFACE_PX <-> BFACE_NX etc.
int  building_face_opposite(int face);

// the four faces that ring a given face (its edge neighbors), written into
// out[4]. used by placement preview + multiface checks.
void building_face_ring(int face, int out[4]);

// is this a vertical face (top/bottom)? plants and slabs care.
int  building_face_is_vertical(int face);

// given the block we hit and the face, fill the adjacent cell coords. this
// is literally "hit + normal" but it shows up enough to deserve a name.
void building_face_adjacent(int hx, int hy, int hz, int face,
                            int *ax, int *ay, int *az);

// pick the face of a unit cube whose normal best opposes `dir`. used when we
// need a sensible face but the raycaster didn't give one (e.g. self-place).
int  building_face_from_dir(vec3 dir);

// center of a given face of the block at (x,y,z), in world space. used for
// placement particles / sound origin.
vec3 building_face_center(int x, int y, int z, int face);

#endif
