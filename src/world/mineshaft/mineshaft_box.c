#include "mineshaft_box.h"

mineshaft_box mineshaft_box_make(int x0, int y0, int z0, int x1, int y1, int z1) {
    mineshaft_box b = { x0, y0, z0, x1, y1, z1 };
    return b;
}

mineshaft_box mineshaft_box_at(int x, int y, int z, int w, int h, int d) {
    mineshaft_box b = { x, y, z, x + w, y + h, z + d };
    return b;
}

int mineshaft_box_width (const mineshaft_box *b) { return b->x1 - b->x0; }
int mineshaft_box_height(const mineshaft_box *b) { return b->y1 - b->y0; }
int mineshaft_box_depth (const mineshaft_box *b) { return b->z1 - b->z0; }

int mineshaft_box_volume(const mineshaft_box *b) {
    return mineshaft_box_width(b) * mineshaft_box_height(b) * mineshaft_box_depth(b);
}

mineshaft_box mineshaft_box_translate(mineshaft_box b, int dx, int dy, int dz) {
    b.x0 += dx; b.x1 += dx;
    b.y0 += dy; b.y1 += dy;
    b.z0 += dz; b.z1 += dz;
    return b;
}

mineshaft_box mineshaft_box_inset(mineshaft_box b, int mxz, int my) {
    b.x0 += mxz; b.x1 -= mxz;
    b.z0 += mxz; b.z1 -= mxz;
    b.y0 += my;  b.y1 -= my;
    // dont let it invert into a negative box; clamp to degenerate-but-valid.
    if (b.x1 < b.x0) b.x1 = b.x0;
    if (b.y1 < b.y0) b.y1 = b.y0;
    if (b.z1 < b.z0) b.z1 = b.z0;
    return b;
}

mineshaft_box mineshaft_box_union(mineshaft_box a, mineshaft_box b) {
    if (mineshaft_box_volume(&a) == 0) return b;
    if (mineshaft_box_volume(&b) == 0) return a;
    mineshaft_box r;
    r.x0 = a.x0 < b.x0 ? a.x0 : b.x0;
    r.y0 = a.y0 < b.y0 ? a.y0 : b.y0;
    r.z0 = a.z0 < b.z0 ? a.z0 : b.z0;
    r.x1 = a.x1 > b.x1 ? a.x1 : b.x1;
    r.y1 = a.y1 > b.y1 ? a.y1 : b.y1;
    r.z1 = a.z1 > b.z1 ? a.z1 : b.z1;
    return r;
}

int mineshaft_box_overlaps(const mineshaft_box *a, const mineshaft_box *b) {
    // separating-axis on integer half-open ranges.
    if (a->x1 <= b->x0 || b->x1 <= a->x0) return 0;
    if (a->y1 <= b->y0 || b->y1 <= a->y0) return 0;
    if (a->z1 <= b->z0 || b->z1 <= a->z0) return 0;
    return 1;
}

int mineshaft_box_contains(const mineshaft_box *b, int x, int y, int z) {
    return x >= b->x0 && x < b->x1 &&
           y >= b->y0 && y < b->y1 &&
           z >= b->z0 && z < b->z1;
}

void mineshaft_dir_step(mineshaft_dir d, int *dx, int *dz) {
    switch (d) {
        case MINESHAFT_NORTH: *dx =  0; *dz = -1; break;
        case MINESHAFT_EAST:  *dx =  1; *dz =  0; break;
        case MINESHAFT_SOUTH: *dx =  0; *dz =  1; break;
        case MINESHAFT_WEST:  *dx = -1; *dz =  0; break;
        default:              *dx =  0; *dz =  0; break;
    }
}

mineshaft_dir mineshaft_dir_rot(mineshaft_dir d, int quarters) {
    int v = ((int)d + quarters) & 3;   // mask folds negatives over the 4-cycle
    return (mineshaft_dir)v;
}

mineshaft_dir mineshaft_dir_opposite(mineshaft_dir d) {
    return mineshaft_dir_rot(d, 2);
}

int mineshaft_dir_link_bit(mineshaft_dir d) {
    return 1 << (int)d;
}
