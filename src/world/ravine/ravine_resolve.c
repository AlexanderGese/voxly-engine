#include "ravine_resolve.h"

#include "ravine_profile.h"
#include "ravine_noise.h"
#include <math.h>

ravine_mask_stats ravine_resolve(ravine_field *f, const ravine_path *path,
                                 const ravine_params *p) {
    ravine_mask_stats st = {0, 0, 0, 0, 0};

    // quick reject in world space: does the region box come near the path? the
    // path lives in world coords so a multi-chunk ravine stays one curve.
    int rwx0, rwz0, rwx1, rwz1;
    ravine_field_to_world(f, 0, 0, &rwx0, &rwz0);
    ravine_field_to_world(f, RAVINE_DIM_X - 1, RAVINE_DIM_Z - 1, &rwx1, &rwz1);
    if (!ravine_path_touches(path, (float)rwx0, (float)rwz0,
                             (float)rwx1, (float)rwz1))
        return st;

    for (int z = 0; z < RAVINE_DIM_Z; z++) {
        for (int x = 0; x < RAVINE_DIM_X; x++) {
            int wx, wz;
            ravine_field_to_world(f, x, z, &wx, &wz);

            float dist, hw, sfloor;
            if (!ravine_path_nearest(path, (float)wx, (float)wz,
                                     &dist, &hw, &sfloor))
                continue;

            // ripple the effective distance with a world-space noise field so
            // the wall erodes in and out instead of reading as a perfect offset.
            float ripple = ravine_fbm2((float)wx * 0.08f, (float)wz * 0.08f,
                                       p->seed ^ 0x2c11u, 3, 2.0f, 0.5f);
            float edist = dist + ripple * p->wall_jitter;
            if (edist < 0.0f) edist = 0.0f;

            ravine_profile prof = ravine_profile_eval(edist, hw, p);
            if (prof.kind == RAVINE_OUTSIDE || prof.cut <= 0)
                continue;

            int idx = ravine_cell_index(x, z);
            if (idx < 0) continue;
            int surf = f->surface[idx];

            int floor_from_cut    = surf - prof.cut;
            int floor_from_spline = (int)floorf(sfloor);

            // channel cells trench down to whichever floor is lower; walls and
            // rims just shave from the top and dont chase the spline floor.
            int floor;
            if (prof.kind == RAVINE_FLOOR)
                floor = (floor_from_spline < floor_from_cut)
                            ? floor_from_spline : floor_from_cut;
            else
                floor = floor_from_cut;

            if (floor < p->min_floor_y) floor = p->min_floor_y;
            if (floor >= surf) continue;   // nothing to remove here

            int real_cut = surf - floor;

            // deeper cut wins where ravines overlap.
            if (real_cut <= f->cut[idx]) continue;

            f->cut[idx]     = real_cut;
            f->floor_y[idx] = floor;
            f->kind[idx]    = (uint8_t)prof.kind;
            f->dirty = 1;

            // tally only the centre footprint so neighbours dont double-count
            // the shared pad ring.
            if (x >= RAVINE_PAD && x < RAVINE_PAD + CHUNK_SIZE_X &&
                z >= RAVINE_PAD && z < RAVINE_PAD + CHUNK_SIZE_Z) {
                st.cells_cut++;
                if (prof.kind == RAVINE_WALL)  st.wall_cells++;
                if (prof.kind == RAVINE_FLOOR) st.floor_cells++;
                if (prof.kind == RAVINE_RIM)   st.rim_cells++;
                if (real_cut > st.deepest) st.deepest = real_cut;
            }
        }
    }
    return st;
}
