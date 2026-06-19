#include "lightprop_types.h"
const int LP_DX[6] = { 1, -1,  0,  0,  0,  0 }
;
const int LP_DY[6] = { 0,  0,  1, -1,  0,  0 }
;
const int LP_DZ[6] = { 0,  0,  0,  0,  1, -1 }
;
int lp_transmits(block_id id) {
    // air and anything not flagged opaque lets light through. glass counts.
    return !block_is_opaque(id);
}

int lp_attenuation(block_id id) {
    // base cost of a step is always 1 (applied by the propagator). this returns
    // the EXTRA hit. water/ice are murky, so light dies faster in them.
    switch (id) {
        case BLOCK_WATER: return 2;
case BLOCK_ICE:   return 1;
case BLOCK_LEAVES:return 1;
default:          return 0;
}
}
