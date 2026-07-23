#include "water_foam.h"
#include "water_config.h"
#include "../../util/darray.h"
#include "../../world/block.h"
#include <math.h>
#include <stdlib.h>
#define WATER_FOAM_RESCAN_DIST 8
void water_foam_init(water_foam_set *s, int surface_y, int region) {
    s->points    = NULL;
    s->surface_y = surface_y;
    s->region    = region < 4 ? 4 : region;
    // sentinel that forces the first update to scan
    s->last_cx   = 0x7fffffff;
    s->last_cz   = 0x7fffffff;
}

void water_foam_destroy(water_foam_set *s) {
    darr_free(s->points);
;
int contacts = 0;
for (int i = 0;
i < 4;
}
