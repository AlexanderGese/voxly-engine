#include "structgen_box.h"

structgen_box structgen_box_make(int x0, int y0, int z0, int x1, int y1, int z1) {
    structgen_box b = { x0, y0, z0, x1, y1, z1 };
    return b;
}

structgen_box structgen_box_at(int x, int y, int z, int w, int h, int d) {
    structgen_box b = { x, y, z, x + w, y + h, z + d };
    return b;
}

int structgen_box_width (const structgen_box *b) { return b->x1 - b->x0; }
int structgen_box_height(const structgen_box *b) { return b->y1 - b->y0; }
int structgen_box_depth (const structgen_box *b) { return b->z1 - b->z0; }

int structgen_box_volume(const structgen_box *b) {
    return structgen_box_width(b) * structgen_box_height(b) * structgen_box_depth(b);
}

structgen_box structgen_box_translate(structgen_box b, int dx, int dy, int dz) {
    b.x0 += dx; b.x1 += dx;
    b.y0 += dy; b.y1 += dy;
    b.z0 += dz; b.z1 += dz;
    return b;
}

structgen_box structgen_box_inset(structgen_box b, int mxz, int my) {
    b.x0 += mxz; b.x1 -= mxz;
    b.z0 += mxz; b.z1 -= mxz;
    b.y0 += my;  b.y1 -= my;
    // dont let it invert; clamp to a degenerate but valid box.
    if (b.x1 < b.x0) b.x1 = b.x0;
    if (b.y1 < b.y0) b.y1 = b.y0;
    if (b.z1 < b.z0) b.z1 = b.z0;
    return b;
}

int structgen_box_overlaps(const structgen_box *a, const structgen_box *b) {
    // standard separating-axis on integer half-open ranges.
    if (a->x1 <= b->x0 || b->x1 <= a->x0) return 0;
    if (a->y1 <= b->y0 || b->y1 <= a->y0) return 0;
    if (a->z1 <= b->z0 || b->z1 <= a->z0) return 0;
    return 1;
}

int structgen_box_contains_xz(const structgen_box *b, int x, int z) {
    return x >= b->x0 && x < b->x1 && z >= b->z0 && z < b->z1;
}

void structgen_dir_step(structgen_dir d, int *dx, int *dz) {
    switch (d) {
        case STRUCTGEN_NORTH: *dx =  0; *dz = -1; break;
        case STRUCTGEN_EAST:  *dx =  1; *dz =  0; break;
        case STRUCTGEN_SOUTH: *dx =  0; *dz =  1; break;
        case STRUCTGEN_WEST:  *dx = -1; *dz =  0; break;
        default:              *dx =  0; *dz =  0; break;
    }
}

structgen_dir structgen_dir_rot(structgen_dir d, int quarters) {
    int v = ((int)d + quarters) & 3;   // mask handles negatives for 4-cycle
    return (structgen_dir)v;
}
