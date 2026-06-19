#include "lightprop_step.h"

uint8_t lp_step_falloff(uint8_t level, block_id nid) {
    if (!lp_transmits(nid)) return 0;          // opaque, nothing gets in
    int cost = 1 + lp_attenuation(nid);
    if (level <= cost) return 0;               // would underflow to <=0
    return (uint8_t)(level - cost);
}

int lp_step_is_sky_drop(lp_channel ch, int dir, uint8_t level, block_id nid) {
    if (ch != LP_SKY) return 0;
    if (level != MAX_LIGHT) return 0;          // only full-strength sky falls free
    if (dir != 3) return 0;                    // dir 3 == -y, straight down
    // and only through clear air; tinted media (water) still attenuates.
    if (!lp_transmits(nid)) return 0;
    if (lp_attenuation(nid) != 0) return 0;
    return 1;
}

uint8_t lp_step_propagate(lp_channel ch, int dir, uint8_t level, block_id nid) {
    if (lp_step_is_sky_drop(ch, dir, level, nid)) {
        return MAX_LIGHT;                       // no falloff going down
    }
    return lp_step_falloff(level, nid);
}
