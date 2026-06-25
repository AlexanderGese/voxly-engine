#include "stronghold_box.h"

// nothing clever in here. just the arithmetic, written out so the room builders
// stay readable. inclusive min / exclusive max throughout.

stronghold_box stronghold_box_make(int x0, int y0, int z0, int x1, int y1, int z1) {
    stronghold_box b;
    // normalize so min<=max no matter what the caller passed.
    b.x0 = x0 < x1 ? x0 : x1;  b.x1 = x0 < x1 ? x1 : x0;
    b.y0 = y0 < y1 ? y0 : y1;  b.y1 = y0 < y1 ? y1 : y0;
    b.z0 = z0 < z1 ? z0 : z1;  b.z1 = z0 < z1 ? z1 : z0;
    return b;
}

stronghold_box stronghold_box_at(int x, int y, int z, int w, int h, int d) {
    return stronghold_box_make(x, y, z, x + w, y + h, z + d);
}

int stronghold_box_width (const stronghold_box *b) { return b->x1 - b->x0; }
int stronghold_box_height(const stronghold_box *b) { return b->y1 - b->y0; }
int stronghold_box_depth (const stronghold_box *b) { return b->z1 - b->z0; }

int stronghold_box_volume(const stronghold_box *b) {
    return (b->x1 - b->x0) * (b->y1 - b->y0) * (b->z1 - b->z0);
}

void stronghold_box_center(const stronghold_box *b, int *cx, int *cy, int *cz) {
    if (cx) *cx = b->x0 + (b->x1 - b->x0) / 2;
    if (cy) *cy = b->y0 + (b->y1 - b->y0) / 2;
    if (cz) *cz = b->z0 + (b->z1 - b->z0) / 2;
}

stronghold_box stronghold_box_translate(stronghold_box b, int dx, int dy, int dz) {
    b.x0 += dx; b.x1 += dx;
    b.y0 += dy; b.y1 += dy;
    b.z0 += dz; b.z1 += dz;
    return b;
}

stronghold_box stronghold_box_inset(stronghold_box b, int mxz, int my) {
    b.x0 -= mxz; b.x1 += mxz;
    b.z0 -= mxz; b.z1 += mxz;
    b.y0 -= my;  b.y1 += my;
    return b;
}

stronghold_box stronghold_box_union(stronghold_box a, stronghold_box b) {
    stronghold_box u;
    u.x0 = a.x0 < b.x0 ? a.x0 : b.x0;
    u.y0 = a.y0 < b.y0 ? a.y0 : b.y0;
    u.z0 = a.z0 < b.z0 ? a.z0 : b.z0;
    u.x1 = a.x1 > b.x1 ? a.x1 : b.x1;
    u.y1 = a.y1 > b.y1 ? a.y1 : b.y1;
    u.z1 = a.z1 > b.z1 ? a.z1 : b.z1;
    return u;
}

int stronghold_box_overlaps(const stronghold_box *a, const stronghold_box *b) {
    return a->x0 < b->x1 && a->x1 > b->x0 &&
           a->y0 < b->y1 && a->y1 > b->y0 &&
           a->z0 < b->z1 && a->z1 > b->z0;
}

int stronghold_box_overlaps_pad(const stronghold_box *a, const stronghold_box *b, int pad) {
    // inflate a by pad on all sides, then plain overlap. cheap.
    stronghold_box ap = *a;
    ap.x0 -= pad; ap.y0 -= pad; ap.z0 -= pad;
    ap.x1 += pad; ap.y1 += pad; ap.z1 += pad;
    return stronghold_box_overlaps(&ap, b);
}

int stronghold_box_contains(const stronghold_box *b, int x, int y, int z) {
    return x >= b->x0 && x < b->x1 &&
           y >= b->y0 && y < b->y1 &&
           z >= b->z0 && z < b->z1;
}

int stronghold_box_contains_xz(const stronghold_box *b, int x, int z) {
    return x >= b->x0 && x < b->x1 && z >= b->z0 && z < b->z1;
}

void stronghold_dir_step(stronghold_dir d, int *dx, int *dz) {
    switch (d) {
        case STRONGHOLD_NORTH: *dx =  0; *dz = -1; break;
        case STRONGHOLD_EAST:  *dx =  1; *dz =  0; break;
        case STRONGHOLD_SOUTH: *dx =  0; *dz =  1; break;
        case STRONGHOLD_WEST:  *dx = -1; *dz =  0; break;
        default:               *dx =  0; *dz =  0; break;
    }
}

stronghold_dir stronghold_dir_rot(stronghold_dir d, int quarters) {
    // wrap into [0,4) the slow-but-correct way, negatives included.
    int q = ((int)d + quarters) % 4;
    if (q < 0) q += 4;
    return (stronghold_dir)q;
}

stronghold_dir stronghold_dir_opposite(stronghold_dir d) {
    return stronghold_dir_rot(d, 2);
}
