#include "ambient_occlusion.h"
#include "../world/block.h"
static int is_opaque(world *w, int x, int y, int z) {
    return block_is_opaque(world_get_block(w, x, y, z));
}

// each vertex has two side neighbors and one corner neighbor.
// AO value = 3 - (side1 + side2 + corner) if both sides occluded,
// else 3 - (side1 + side2 + corner)
static float ao_vertex(int side1, int side2, int corner) {
    if (side1 && side2) return 0.0f;
float v = 3.0f - (float)(side1 + side2 + corner);
return v / 3.0f;
}

void ao_face(world *w, int wx, int wy, int wz, int face, float out[4]) {
    // neighbor offsets depend on face direction.
    // for simplicity, just sample the 4 diagonal corners and average.
    // not perfectly accurate but good enough here.

    int s1, s2, c;

    switch (face) {
    case 2: // +Y (top face)
        // vertex 0: (-x, -z)
        s1 = is_opaque(w, wx-1, wy+1, wz);
        s2 = is_opaque(w, wx, wy+1, wz-1);
        c  = is_opaque(w, wx-1, wy+1, wz-1);
        out[0] = ao_vertex(s1, s2, c);
        // vertex 1: (-x, +z)
        s1 = is_opaque(w, wx-1, wy+1, wz);
        s2 = is_opaque(w, wx, wy+1, wz+1);
        c  = is_opaque(w, wx-1, wy+1, wz+1);
        out[1] = ao_vertex(s1, s2, c);
        // vertex 2: (+x, +z)
        s1 = is_opaque(w, wx+1, wy+1, wz);
        s2 = is_opaque(w, wx, wy+1, wz+1);
        c  = is_opaque(w, wx+1, wy+1, wz+1);
        out[2] = ao_vertex(s1, s2, c);
        // vertex 3: (+x, -z)
        s1 = is_opaque(w, wx+1, wy+1, wz);
        s2 = is_opaque(w, wx, wy+1, wz-1);
        c  = is_opaque(w, wx+1, wy+1, wz-1);
        out[3] = ao_vertex(s1, s2, c);
        break;

    case 3: // -Y (bottom face)
        s1 = is_opaque(w, wx-1, wy-1, wz);
        s2 = is_opaque(w, wx, wy-1, wz+1);
        c  = is_opaque(w, wx-1, wy-1, wz+1);
        out[0] = ao_vertex(s1, s2, c);
        s1 = is_opaque(w, wx-1, wy-1, wz);
        s2 = is_opaque(w, wx, wy-1, wz-1);
        c  = is_opaque(w, wx-1, wy-1, wz-1);
        out[1] = ao_vertex(s1, s2, c);
        s1 = is_opaque(w, wx+1, wy-1, wz);
        s2 = is_opaque(w, wx, wy-1, wz-1);
        c  = is_opaque(w, wx+1, wy-1, wz-1);
        out[2] = ao_vertex(s1, s2, c);
        s1 = is_opaque(w, wx+1, wy-1, wz);
        s2 = is_opaque(w, wx, wy-1, wz+1);
        c  = is_opaque(w, wx+1, wy-1, wz+1);
        out[3] = ao_vertex(s1, s2, c);
        break;

    default:
        // side faces: simplified — just use 0.85 everywhere
        out[0] = out[1] = out[2] = out[3] = 0.85f;
        break;
    }
}

float ao_factor(world *w, int wx, int wy, int wz,
                int face, int vertex_index) {
    float corners[4];
ao_face(w, wx, wy, wz, face, corners);
if (vertex_index < 0 || vertex_index > 3) return 1.0f;
return corners[vertex_index];
}
