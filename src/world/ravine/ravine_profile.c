#include "ravine_profile.h"

#include <math.h>

float ravine_profile_wall_run(const ravine_params *p) {
    // wall_slope is run-per-rise, so a max_depth wall runs this far horizontally.
    float run = (float)p->max_depth * p->wall_slope;
    if (run < 1.0f) run = 1.0f;
    return run;
}

ravine_profile ravine_profile_eval(float dist, float half_width,
                                   const ravine_params *p) {
    ravine_profile prof;
    prof.cut = 0;
    prof.kind = RAVINE_OUTSIDE;
    prof.wall_t = 0.0f;

    float wall_run = ravine_profile_wall_run(p);
    float wall_r   = half_width + wall_run;   // outer edge of the steep wall
    float rim_r    = wall_r + 2.5f;           // short feathered lip past it

    if (dist <= half_width) {
        // flat channel bottom: the full cut.
        prof.cut = p->max_depth;
        prof.kind = RAVINE_FLOOR;
        return prof;
    }

    if (dist <= wall_r) {
        // the wall. depth ramps from full at the floor edge to zero at the outer
        // wall radius. a slight power bias keeps the upper wall steep so it reads
        // as a cliff rather than a ramp. wall_t = 0 at floor edge, 1 at the lip.
        float u = (dist - half_width) / (wall_r - half_width + 1e-6f);
        float drop = 1.0f - powf(u, 1.7f);
        int cut = (int)lroundf((float)p->max_depth * drop);
        if (cut < 1) cut = 1;
        prof.cut = cut;
        prof.kind = RAVINE_WALL;
        prof.wall_t = u;
        return prof;
    }

    if (dist <= rim_r) {
        // the rim: a shallow shave that eases the lip to ground level. cosmetic.
        float u = (dist - wall_r) / (rim_r - wall_r + 1e-6f);
        float shave = (1.0f - u);
        int cut = (int)lroundf(shave * 2.0f);
        if (cut <= 0) return prof;   // nothing worth cutting, leave OUTSIDE
        prof.cut = cut;
        prof.kind = RAVINE_RIM;
        return prof;
    }

    // beyond the rim: untouched.
    return prof;
}
