#include "pf_debug.h"
#include "pf_smooth.h"
#include "../../util/log.h"

#include <math.h>
#include <stdlib.h>

// recover the block coord a waypoint center sits in. center_of() in pf.c
// puts the point at feet height (floor + 1), so subtract that back out.
static pf_coord wp_block(vec3 p) {
    return pf_coord_make((int)floorf(p.x),
                         (int)floorf(p.y) - 1,
                         (int)floorf(p.z));
}

pf_path_stats pf_debug_stats(pf_grid *g, const pf_path *path) {
    pf_path_stats st;
    st.waypoints         = path->count;
    st.total_length      = 0.0f;
    st.direction_changes = 0;
    st.max_step_up       = 0;
    st.max_step_down     = 0;
    st.valid             = 1;

    float pdx = 0.0f, pdz = 0.0f;
    int have_prev_dir = 0;

    for (int i = 1; i < path->count; i++) {
        vec3 a = path->pts[i - 1].pos;
        vec3 b = path->pts[i].pos;

        float dx = b.x - a.x;
        float dy = b.y - a.y;
        float dz = b.z - a.z;
        st.total_length += sqrtf(dx * dx + dy * dy + dz * dz);

        int step = (int)lroundf(dy);
        if (step > st.max_step_up)   st.max_step_up = step;
        if (-step > st.max_step_down) st.max_step_down = -step;

        // count a direction change when the xz heading flips noticeably.
        float len = sqrtf(dx * dx + dz * dz);
        if (len > 1e-4f) {
            float ndx = dx / len, ndz = dz / len;
            if (have_prev_dir) {
                float dot = ndx * pdx + ndz * pdz;
                if (dot < 0.98f) st.direction_changes++;
            }
            pdx = ndx; pdz = ndz;
            have_prev_dir = 1;
        }

        // validity: the segment between the two block columns must be clear.
        pf_coord ca = wp_block(a);
        pf_coord cb = wp_block(b);
        if (!pf_smooth_line_clear(g, ca, cb)) st.valid = 0;
    }
    return st;
}

int pf_debug_validate(pf_grid *g, const pf_path *path) {
    if (path->count == 0) return 0;

    for (int i = 0; i < path->count; i++) {
        pf_coord c = wp_block(path->pts[i].pos);
        int lx, lz, fl;
        if (!pf_grid_to_local(g, c, &lx, &lz)) return 0;
        if (!pf_grid_standable(g, lx, lz, c.y, &fl)) return 0;
    }
    for (int i = 1; i < path->count; i++) {
        pf_coord a = wp_block(path->pts[i - 1].pos);
        pf_coord b = wp_block(path->pts[i].pos);
        if (!pf_smooth_line_clear(g, a, b)) return 0;
    }
    return 1;
}

void pf_debug_dump(const pf_path *path) {
    LOGD("pf path: %d waypoints, cursor=%d, found=%d",
         path->count, path->cursor, path->found);
    for (int i = 0; i < path->count; i++) {
        vec3 p = path->pts[i].pos;
        LOGD("  [%2d] (%.1f, %.1f, %.1f)%s", i, p.x, p.y, p.z,
             i == path->cursor ? "  <- cursor" : "");
    }
}

void pf_debug_dump_grid(pf_grid *g, pf_coord start, pf_coord goal) {
    // top-down ascii of the window. one char per column. big but it's debug.
    char line[PF_WINDOW_SIZE + 1];
    LOGD("pf grid window origin (%d,%d,%d):",
         g->origin.x, g->origin.y, g->origin.z);

    int slx = -1, slz = -1, glx = -1, glz = -1;
    pf_grid_to_local(g, start, &slx, &slz);
    pf_grid_to_local(g, goal,  &glx, &glz);

    for (int z = 0; z < PF_WINDOW_SIZE; z++) {
        for (int x = 0; x < PF_WINDOW_SIZE; x++) {
            int fl;
            char ch;
            if (x == slx && z == slz)      ch = 'S';
            else if (x == glx && z == glz) ch = 'G';
            else if (pf_grid_standable(g, x, z, g->origin.y, &fl)) ch = '.';
            else ch = '#';
            line[x] = ch;
        }
        line[PF_WINDOW_SIZE] = '\0';
        LOGD("%s", line);
    }
}
