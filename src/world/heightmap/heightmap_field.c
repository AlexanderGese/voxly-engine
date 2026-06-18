#include "heightmap_field.h"

static int clamp_local(int v) {
    // clamp a local coord into -PAD..15+PAD so the read helpers stay in the grid
    int lo = -HEIGHTMAP_PAD;
    int hi = 15 + HEIGHTMAP_PAD;
    return v < lo ? lo : (v > hi ? hi : v);
}

void heightmap_field_build(heightmap_field *f, const heightmap_params *p,
                           int cx_world, int cz_world) {
    f->ox = cx_world;
    f->oz = cz_world;

    for (int lz = -HEIGHTMAP_PAD; lz <= 15 + HEIGHTMAP_PAD; lz++) {
        for (int lx = -HEIGHTMAP_PAD; lx <= 15 + HEIGHTMAP_PAD; lx++) {
            int wx = cx_world + lx;
            int wz = cz_world + lz;

            heightmap_column col;
            heightmap_column_resolve(p, wx, wz, &col);

            int i = heightmap_field_idx(lx, lz);
            f->surface[i]    = col.surface_y;
            f->steepness[i]  = col.steepness;
            f->underwater[i] = (uint8_t)col.underwater;
        }
    }
}

int heightmap_field_at(const heightmap_field *f, int lx, int lz) {
    return f->surface[heightmap_field_idx(clamp_local(lx), clamp_local(lz))];
}

float heightmap_field_steep_at(const heightmap_field *f, int lx, int lz) {
    return f->steepness[heightmap_field_idx(clamp_local(lx), clamp_local(lz))];
}

void heightmap_field_minmax(const heightmap_field *f, int *out_min, int *out_max) {
    int mn = 1 << 30, mx = -(1 << 30);
    // interior only, the skirt belongs to the neighbours
    for (int lz = 0; lz < 16; lz++) {
        for (int lx = 0; lx < 16; lx++) {
            int h = f->surface[heightmap_field_idx(lx, lz)];
            if (h < mn) mn = h;
            if (h > mx) mx = h;
        }
    }
    if (out_min) *out_min = mn;
    if (out_max) *out_max = mx;
}
