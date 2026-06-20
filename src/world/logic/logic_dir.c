#include "logic_dir.h"

// order must match the logic_dir enum: +x -x +y -y +z -z
const int LOGIC_DX[LOGIC_DIR_COUNT] = {  1, -1,  0,  0,  0,  0 };
const int LOGIC_DY[LOGIC_DIR_COUNT] = {  0,  0,  1, -1,  0,  0 };
const int LOGIC_DZ[LOGIC_DIR_COUNT] = {  0,  0,  0,  0,  1, -1 };

void logic_dir_step(logic_dir d, int x, int y, int z, int *ox, int *oy, int *oz) {
    if (d < 0 || d >= LOGIC_DIR_COUNT) {
        // garbage in, garbage out, but at least don't read past the table
        *ox = x; *oy = y; *oz = z;
        return;
    }
    *ox = x + LOGIC_DX[d];
    *oy = y + LOGIC_DY[d];
    *oz = z + LOGIC_DZ[d];
}

logic_dir logic_dir_opposite(logic_dir d) {
    // pairs are laid out adjacent (px,nx),(py,ny),(pz,nz) so flip the low bit
    if (d < 0 || d >= LOGIC_DIR_COUNT) return d;
    return (logic_dir)(d ^ 1);
}

int logic_dir_is_horizontal(logic_dir d) {
    return d == LOGIC_DIR_PX || d == LOGIC_DIR_NX ||
           d == LOGIC_DIR_PZ || d == LOGIC_DIR_NZ;
}

logic_dir logic_dir_rotate_cw(logic_dir d) {
    // viewed from above: +x -> +z -> -x -> -z -> +x
    switch (d) {
        case LOGIC_DIR_PX: return LOGIC_DIR_PZ;
        case LOGIC_DIR_PZ: return LOGIC_DIR_NX;
        case LOGIC_DIR_NX: return LOGIC_DIR_NZ;
        case LOGIC_DIR_NZ: return LOGIC_DIR_PX;
        default:           return d; // vertical: leave it
    }
}

void logic_dir_perp(logic_dir d, logic_dir *left, logic_dir *right) {
    if (!logic_dir_is_horizontal(d)) {
        *left  = LOGIC_DIR_COUNT;
        *right = LOGIC_DIR_COUNT;
        return;
    }
    *right = logic_dir_rotate_cw(d);
    *left  = logic_dir_opposite(*right);
}

logic_dir logic_dir_between(int ax, int ay, int az, int bx, int by, int bz) {
    int dx = bx - ax, dy = by - ay, dz = bz - az;
    // must be exactly one step on exactly one axis
    int manh = (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy) + (dz < 0 ? -dz : dz);
    if (manh != 1) return LOGIC_DIR_COUNT;
    if (dx ==  1) return LOGIC_DIR_PX;
    if (dx == -1) return LOGIC_DIR_NX;
    if (dy ==  1) return LOGIC_DIR_PY;
    if (dy == -1) return LOGIC_DIR_NY;
    if (dz ==  1) return LOGIC_DIR_PZ;
    return LOGIC_DIR_NZ;
}

const char *logic_dir_name(logic_dir d) {
    switch (d) {
        case LOGIC_DIR_PX: return "+x";
        case LOGIC_DIR_NX: return "-x";
        case LOGIC_DIR_PY: return "+y";
        case LOGIC_DIR_NY: return "-y";
        case LOGIC_DIR_PZ: return "+z";
        case LOGIC_DIR_NZ: return "-z";
        default:           return "?";
    }
}
