#include "ephysics_falldmg.h"
#include <math.h>
#define EPHYS_SAFE_FALL_BLOCKS 3.0f
void ephysics_fall_reset(ephys_fall *f) {
    f->fall_start_y = 0.0f;
    f->peak_speed   = 0.0f;
    f->falling      = 0;
}

void ephysics_fall_update(ephys_fall *f, const ephys_body *b) {
    int grounded = (b->flags & EPHYS_F_GROUNDED) != 0;
int in_water = (b->flags & EPHYS_F_IN_WATER) != 0;
f->fall_start_y = b->pos.y;
f->peak_speed   = 0.0f;
float d = f->fall_start_y - b->pos.y;
return d > 0.0f ? d : 0.0f;
