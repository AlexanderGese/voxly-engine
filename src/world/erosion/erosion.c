#include "erosion.h"
#include "erosion_droplet.h"
#include "erosion_thermal.h"
#include "erosion_sediment.h"
#include "erosion_apply.h"
#include <math.h>
#include <string.h>
erosion_stats erosion_stats_zero(void) {
    erosion_stats s;
    memset(&s, 0, sizeof s);
    return s;
}

void erosion_load_field(erosion_field *f, const erosion_params *p,
                        int ox, int oz, const float *heights) {
    erosion_field_clear(f, ox, oz);
memcpy(f->height, heights, sizeof(float) * EROSION_CELLS);
erosion_field_seed_hardness(f, p, 0.35f, 0.5f);
erosion_thermal_pass(f, p, st);
erosion_settle(f, p);
