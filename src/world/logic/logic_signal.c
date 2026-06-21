#include "logic_signal.h"

uint8_t logic_clamp(int p) {
    if (p < LOGIC_MIN) return LOGIC_MIN;
    if (p > LOGIC_MAX) return LOGIC_MAX;
    return (uint8_t)p;
}

uint8_t logic_attenuate(uint8_t p) {
    // one off per step, floored at zero. (don't underflow a uint8.)
    return p > 0 ? (uint8_t)(p - 1) : 0;
}

uint8_t logic_attenuate_n(uint8_t p, int n) {
    if (n <= 0) return p;
    int r = (int)p - n;
    return logic_clamp(r);
}

uint8_t logic_max2(uint8_t a, uint8_t b) {
    return a > b ? a : b;
}

int logic_is_on(uint8_t p) {
    return p > 0;
}

uint8_t logic_invert(uint8_t p) {
    return p > 0 ? 0 : LOGIC_MAX;
}

uint8_t logic_restore(uint8_t p) {
    return p > 0 ? LOGIC_MAX : 0;
}
