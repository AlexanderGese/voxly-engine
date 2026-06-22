#include "ravine_detail.h"

#include "ravine_strata.h"
#include "ravine_noise.h"
#include "ravine_carve.h"
#include "../block.h"
#include <math.h>
#include <stddef.h>

// the hardness threshold either side of which a band juts or crumbles. picked so
// stone (200) and cobble (160) tend to jut while dirt (70) and sand (40) recede.
#define DETAIL_HARD   150
#define DETAIL_SOFT    90

// is there open space horizontally adjacent to (lx,y,lz)? a wall block only
// reads as a "face" worth roughening if the canyon air is next to it.
static int has_air_neighbor(const chunk *c, int lx, int y, int lz) {
    static const int ox[4] = { 1, -1, 0, 0 };
    static const int oz[4] = { 0, 0, 1, -1 };
    for (int i = 0; i < 4; i++) {
        int nx = lx + ox[i], nz = lz + oz[i];
        if (nx < 0 || nx >= CHUNK_SIZE_X || nz < 0 || nz >= CHUNK_SIZE_Z)
            continue;
        if (chunk_get_block(c, nx, y, nz) == BLOCK_AIR) return 1;
    }
    return 0;
}

// find the open neighbour direction (first air cell horizontally). writes the
// offset into *dx,*dz and returns 1, or 0 if the block is fully walled in.
static int air_dir(const chunk *c, int lx, int y, int lz, int *dx, int *dz) {
    static const int ox[4] = { 1, -1, 0, 0 };
    static const int oz[4] = { 0, 0, 1, -1 };
    for (int i = 0; i < 4; i++) {
        int nx = lx + ox[i], nz = lz + oz[i];
        if (nx < 0 || nx >= CHUNK_SIZE_X || nz < 0 || nz >= CHUNK_SIZE_Z)
            continue;
        if (chunk_get_block(c, nx, y, nz) == BLOCK_AIR) {
            *dx = ox[i]; *dz = oz[i];
            return 1;
        }
    }
    return 0;
}

ravine_detail_stats ravine_detail_apply(chunk *c, const ravine_field *f,
                                        const ravine_strata *strata,
                                        const ravine_params *p) {
    ravine_detail_stats st = {0, 0, 0};

    for (int lz = 0; lz < CHUNK_SIZE_Z; lz++) {
        for (int lx = 0; lx < CHUNK_SIZE_X; lx++) {
            int idx = ravine_cell_index(lx + RAVINE_PAD, lz + RAVINE_PAD);
            if (idx < 0) continue;
            if (f->cut[idx] <= 0) continue;
            if (f->kind[idx] != RAVINE_WALL) continue;   // only walls roughen

            int floor = f->floor_y[idx];
            int surf  = f->surface[idx];
            if (floor >= surf) continue;

            int wx, wz;
            ravine_field_to_world(f, lx + RAVINE_PAD, lz + RAVINE_PAD, &wx, &wz);

            // walk the exposed face from the floor up to the surface. each solid
            // block that fronts onto air is a candidate for jut/shave by its
            // band hardness.
            int top = surf;
            if (top > CHUNK_SIZE_Y - 1) top = CHUNK_SIZE_Y - 1;
            for (int y = floor; y < top; y++) {
                block_id here = chunk_get_block(c, lx, y, lz);
                if (here == BLOCK_AIR || here == BLOCK_BEDROCK) continue;
                if (!has_air_neighbor(c, lx, y, lz)) continue;

                const ravine_band *band = ravine_strata_band(strata, y);
                int hardness = band ? band->hardness : 128;

                // a roughening gate so not every face block moves — keeps the
                // erosion patchy. noise in [0,1], threshold ~0.45.
                float g = ravine_value2((float)wx * 0.35f + (float)y * 0.11f,
                                        (float)wz * 0.35f, p->seed ^ 0x6e21u);
                g = g * 0.5f + 0.5f;
                if (g < 0.45f) continue;

                int dx, dz;
                if (hardness >= DETAIL_HARD) {
                    // jut: push a copy of this rock one block out into the air.
                    if (air_dir(c, lx, y, lz, &dx, &dz)) {
                        int nx = lx + dx, nz = lz + dz;
                        chunk_set_block(c, nx, y, nz, here);
                        st.juts++;
                    }
                } else if (hardness <= DETAIL_SOFT) {
                    // crumble: shave this soft block back to air, and maybe drop
                    // a scree block onto the floor below it.
                    if (ravine_carve_is_diggable(here)) {
                        chunk_set_block(c, lx, y, lz, BLOCK_AIR);
                        st.shaves++;

                        // settle debris: first solid below becomes cobble, but
                        // only right at the floor so we dont fill the channel.
                        if (floor >= 0 && floor < CHUNK_SIZE_Y) {
                            int fy = floor;
                            block_id under = chunk_get_block(c, lx, fy, lz);
                            if (under != BLOCK_AIR && under != BLOCK_BEDROCK &&
                                under != BLOCK_COBBLE && (g > 0.7f)) {
                                chunk_set_block(c, lx, fy, lz, BLOCK_COBBLE);
                                st.scree++;
                            }
                        }
                    }
                }
            }
        }
    }
    return st;
}
